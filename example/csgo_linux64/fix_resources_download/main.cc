/*
    This file is part of Reverse Engine.

    Fix for https://github.com/ValveSoftware/csgo-osx-linux/issues/11
    that allows you to download resources from community servers.
    UPD: #11 issue fixed by Volvo

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <iomanip>
#include <chrono>
#include <type_traits>
#include <zconf.h>
#include <functional>
#include <cmath>
#include <cassert>
#include <sys/wait.h>
#include <curl/curl.h>
#include <reverseengine/core.hh>
#include <reverseengine/value.hh>
#include <reverseengine/scanner.hh>


using namespace std;
using namespace std::chrono;
using namespace std::literals;

const string target = "csgo_linux64";
const string delimeter = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
const char *DOWNLOADMANAGER_SIGNATURE = "\x55\x48\x8D\x3D\x00\x00\x00\x00\x48\x89\xE5\x5D\xE9\xBF\xFF\xFF\xFF";
const char *DOWNLOADMANAGER_MASK = "xxxx????xxxxxxxxx";
bool nowrite = false, nobz2 = false;
int afterDelay = 10;

inline bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
// https://stackoverflow.com/a/27172926/3303059
// I am not satisfied with this "solution" at all!
static int exec_prog(const char *prog, const char *arg1, const char *arg2) {
    pid_t   my_pid;
    int     status, timeout;

    if (0 == (my_pid = fork())) {
        if (-1 == execlp(prog, prog, arg1, arg2, NULL)) {
            perror("child process execve failed [%m]");
            return -1;
        }
    }
    timeout = 20000;

    while (0 == waitpid(my_pid, &status, WNOHANG)) {
        if ( --timeout < 0 ) {
            perror("timeout");
            return -1;
        }
        usleep(500);
    }

    return 0;
}


int
main(int argc, char* argv[])
{
    if (argc > 0)
        nowrite = false;
    if (getuid() > 0) {
        cerr<<"Warning: non root user"<<endl;
    }
    /// Trainer and scanner example
    RE::handler csgo;
    RE::region *engine_client_so;

stage_waiting:;
    cout<<"1. Waiting for process."<<endl;
    cout<<"process: "<<target<<endl;
    for(;;) {
        csgo.attach(target);
        if (csgo.is_good())
            break;
        usleep(500'000);
    }
    cout<<"2. Found!"<<endl;
    cout<<"pid: "<<csgo.pid<<endl;
    cout<<"title: "<<csgo.title<<endl;
    cout<<"path: "<<csgo.get_path()<<endl;
    cout<<"working_directory: "<<csgo.get_working_directory()<<endl;
    cout<<delimeter<<endl;

stage_updating:;
    cout<<"1. Updating regions."<<endl;
    for(;;) {
        csgo.update_regions();
        engine_client_so = csgo.get_region_by_name("engine_client.so");
        if (engine_client_so->is_good())
            break;
        if (!csgo.is_running())
            goto stage_waiting;
        usleep(500'000);
    }
    cout<<"Regions added: "<<csgo.regions.size()<<", ignored: "<<csgo.regions_ignored.size()<<endl;
//    cout<<"Found region: "<<*engine_client_so<<endl;
    cout<<delimeter<<endl;

    //fixme[high]: ?????????????????????????
    std::string moddir = csgo.get_path().string().substr(0, csgo.get_path().string().find_last_of("/\\")) + "/csgo";
    struct stat moddirstat;
    stat(moddir.c_str(), &moddirstat);

    uintptr_t foundDownloadManagerMov{};
    if (!csgo.find_pattern(&foundDownloadManagerMov,
                           engine_client_so,
                           DOWNLOADMANAGER_SIGNATURE,
                           DOWNLOADMANAGER_MASK))
    {
        clog<<"Not found :("<<endl;
        return 1;
    }
    foundDownloadManagerMov += 1;
    cout << ">>> found TheDownloadManager mov: "<<RE::HEX(foundDownloadManagerMov)<<endl;

    uintptr_t downloadManager = csgo.get_absolute_address(foundDownloadManagerMov, 3, 7);
    cout << ">>> Address of TheDownloadManager: "<<RE::HEX(downloadManager)<<endl;

    uintptr_t m_activeRequest, m_queuedRequests_List, curReq;
    int m_queuedRequests_Count, one = 1, httpdone = 2, httperror = 4, m_activeRequest_status = httperror;
    bool bAsHTTP;
    char baseURLbuf[PATH_MAX], gamePathbuf[PATH_MAX];

    CURL *curl;
    FILE *fp;
    CURLcode res;
    char curlerr[CURL_ERROR_SIZE];
    curl = curl_easy_init();
    unsigned long responseCode;
    if(!curl) {
        cout << "Failed to initialize cURL!" << endl;
        exit(1);
    }

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlerr);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    // Error [22]: HTTP response code said error
    // The requested URL returned error: 404 Not Found


    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
    curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Half-Life 2");

    while (csgo.is_running()) {
        sleep(1);
        size_t copied = csgo.read(downloadManager + 4u * 8u, &m_activeRequest, sizeof(m_activeRequest));
        if (copied != sizeof(unsigned long)) {
            cout<<"copied: "<<copied<<endl;
            cout << "copy failed: " << std::strerror(errno) << '\n';
            continue;
        }

        if (csgo.read(downloadManager, &m_queuedRequests_List, sizeof(unsigned long)) != sizeof(unsigned long))
            continue;

        if (csgo.read(downloadManager + 2 * 8, &m_queuedRequests_Count, sizeof(unsigned long)) != sizeof(unsigned long))
            printf("Cannot read utils");

        void** requestList = (void**) malloc((m_queuedRequests_Count + 1) * sizeof(void*));
        *requestList = (void*) m_activeRequest;
        csgo.read(m_queuedRequests_List, requestList + 1, m_queuedRequests_Count * sizeof(void*));

        for(int i = 0; i < m_queuedRequests_Count+1; ++i) {
            curReq = (unsigned long)(requestList[i]);
            csgo.read(curReq + 20, &baseURLbuf, sizeof(baseURLbuf));
            csgo.read(curReq + 532, &gamePathbuf, sizeof(gamePathbuf));
            printf("heheeeey! #%d/%d: %s%s\n", i, m_queuedRequests_Count, baseURLbuf, gamePathbuf);
        }

        //cout << std::dec << m_queuedRequests_Count << endl;

        m_activeRequest_status = httperror;

        for(int i = 0; i < m_queuedRequests_Count+1; ++i) {
            csgo.read(downloadManager + 4 * 8, &m_activeRequest, sizeof(unsigned long));
            if(!m_activeRequest)
                break;

            if(!nowrite)
                csgo.write(m_activeRequest + 8, &httperror, sizeof(int));
            // Process the active request first, then proceed further. If we write status to the active request,
            // the vector gets shifted and we will skip some files.
            curReq = (unsigned long)(requestList[i]);

            csgo.read(curReq + 3, &bAsHTTP, sizeof(bool));
            if(!bAsHTTP)
                continue;

            csgo.read(curReq + 20, &baseURLbuf, sizeof(baseURLbuf));
            csgo.read(curReq + 532, &gamePathbuf, sizeof(gamePathbuf));
            std::string baseURL(baseURLbuf);
            std::string gamePath(gamePathbuf);
            std::string downloadURL = baseURL + gamePath;
            std::string saveURL = moddir + "/" + gamePath;
            std::string gamePathWithBZ2 = gamePath + ".bz2";
            std::string downloadDir = saveURL.substr(0, saveURL.find_last_of("/\\"));

            if (file_exists(saveURL)) {
                cout<<"#"<<i<<"/"<<m_queuedRequests_Count<<" file exists: "<<gamePath<<endl;
                goto SKIPLOOP;
            }
            cout<<"#"<<i<<"/"<<m_queuedRequests_Count<<" downloading: "<<downloadURL<<endl;

            // might as well do it the ghetto way - we're on Linux, so this is available
            exec_prog("mkdir", "-p", downloadDir.c_str());
            fp = fopen(saveURL.c_str(), "wb");
            if(fp == NULL) {
                cout << "Failed to fopen " << saveURL << endl;
                //if(i >= 0)
                if(!nowrite)
                    csgo.write(curReq + 8, &httperror, sizeof(int));
                continue;
            }

            curl_easy_setopt(curl, CURLOPT_URL, downloadURL.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            fclose(fp);

            // HTTP/1.1 200 OK
            if(responseCode != 200 || res) {
                cout << "Error [" << dec << res << " / HTTP: " << responseCode << "]: " << curl_easy_strerror(res) << endl;
                if(strlen(curlerr))
                    cout << curlerr << endl;
                unlink(saveURL.c_str());
                goto SKIPLOOP;
            }

            if(!nobz2) {
                char *ext = strrchr(gamePathbuf, '.');
                if(ext && !strcmp(ext, ".bz2")) {
                    // The file is a BZ2 archive, we can unpack it and just skip the next download.
                    exec_prog("bzip2", "-dk", saveURL.c_str());
                }
            }

            SKIPLOOP:
            if(!res) {
                chown(saveURL.c_str(), moddirstat.st_uid, moddirstat.st_gid);
            }

            if(!nowrite)
                csgo.write(curReq + 8, &httpdone, sizeof(int));

        }

        free(requestList);
        if(nowrite) {
            sleep(afterDelay);
            continue;
        }

        // in case we're downloading faster than CSGO's updating the thread
        usleep(200000);
        csgo.read(downloadManager + 2 * 8, &m_queuedRequests_Count, sizeof(unsigned long));
        // cout << m_queuedRequests_Count << endl;
        for (int i = 0; i < m_queuedRequests_Count + 1; ++i) {
            csgo.read(downloadManager + 4 * 8, &m_activeRequest, sizeof(unsigned long));
            csgo.write(m_activeRequest + 8, &httperror, sizeof(int));
            // cout << i << endl;
            usleep(50000);
        }

        /*while (m_queuedRequests_Count) {
            for (int i = 0; i < m_queuedRequests_Count; ++i) {
                curReq = (unsigned long)(requestList[i]);
                csgo.Write((void*) (curReq + 8), &httpdone, sizeof(int));
            }
            cout << m_queuedRequests_Count << endl;
            csgo.Read((void*) (downloadManager + 2 * 8), &m_queuedRequests_Count, sizeof(unsigned long));
            csgo.Read((void*) (downloadManager + 4 * 8), &m_activeRequest, sizeof(unsigned long));
            if(!m_activeRequest)
                break;
            csgo.Write((void*) (m_activeRequest + 8), &httpdone, sizeof(int));
            usleep(50000);
        }*/

        //csgo.Write((void*) (m_activeRequest + 8), &m_activeRequest_status, sizeof(int));
    }

    curl_easy_cleanup(curl);

    cout << "Game ended." << endl;
    return 0;
}
