/*
    This file is part of Reverse Engine.

    Find process by PID or title, access it's address space, change any
    value you need.

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This library is free software: you can redistribute it and/or modify
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

#ifndef RE_CORE_HH
#define RE_CORE_HH

#include <sys/uio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <iomanip>
#include <algorithm>
#include <dirent.h> //#include <filesystem>
#include "value.hh"
#include "external.hh"
#include "fix_enum.hh"


//namespace fs = std::filesystem;
namespace RE {

class Handle {
public:
    /// Variables
    pid_t pid;
    std::string title;
    std::vector<Cregion> regions_ignored;
    std::vector<Cregion> regions;

    void
    attach(pid_t pid) {
        using namespace std;
        this->pid = pid;
        string path = get_path();
        if (path.empty()) {
            this->pid = 0;
            return;
        }

        size_t exe_cursor_end = path.find_last_of('/');
        if (exe_cursor_end == string::npos) {
            this->pid = 0;
            return;
        }

        this->title = path.substr(exe_cursor_end + 1);
    }

    // FIXME [very low]: incredibly large function
    void
    attach(const std::string& title) {
        using namespace std;
        if (title.empty()) {
            pid = 0;
            return;
        }

        struct dirent *dire;
        DIR *dir = opendir("/proc/");
        if (!dir) {
            pid = 0;
            return;
        }
        while ((dire = readdir(dir)) != NULL) {
            if (dire->d_type != DT_DIR)
                continue;
            string nn(dire->d_name);

            string mapsPath = "/proc/" + nn + "/maps";
            if (access(mapsPath.c_str(), F_OK) == -1)
                continue;

            /// Check to see if the string is numeric (no negatives or dec allowed, which makes this function usable)
            if (strspn(nn.c_str(), "0123456789") == nn.size()) {
                istringstream buffer(nn);
                buffer >> pid;
            } else {
                pid = 0;
            }

            if (!is_good())
                continue;

            string path = get_path();
            if (path.empty())
                continue;

            size_t exe_cursor_end = path.find_last_of('/');
            if (exe_cursor_end == string::npos)
                continue;

            string exe = path.substr(exe_cursor_end + 1);
            if (exe == title) {  /// success
                this->title = title;
                closedir(dir);
                return;
            }
        }
        closedir(dir);
        pid = 0;
    }


    Handle() : cached(*this) {
        this->pid = 0;
    }

    explicit Handle(pid_t pid) : cached(*this) {
        attach(pid);
    }

    explicit Handle(const std::string& title) : cached(*this) {
        attach(title);
    }


    /// Functions
    std::string
    get_symbolic_link_target(const std::string& target) {
        static char buf[PATH_MAX];
        static ssize_t len;
        len = readlink(target.c_str(), buf, sizeof(buf) - 1);
        if (len < 0)
            return "";
        buf[len] = '\0';
        return std::string(buf);
    }

    std::string
    inline __attribute__((always_inline))
    get_path() {
        return get_symbolic_link_target("/proc/" + std::to_string(pid) + "/exe");
    }

    std::string
    inline __attribute__((always_inline))
    get_working_directory() {
        return get_symbolic_link_target("/proc/" + std::to_string(pid) + "/cwd");
    }

    /// Checking
    bool
    inline __attribute__((always_inline))
    is_valid() {
        return pid != 0;
    }

    bool
    inline __attribute__((always_inline))
    is_running() {
        using namespace std;
        static struct stat sts{};
        errno = 0;
        return !(stat(("/proc/" + to_string(pid)).c_str(), &sts) == -1 && errno == ENOENT);
    }

    bool
    inline __attribute__((always_inline))
    is_good() {
        return is_valid() && is_running();
    }

    /// Read_from/write_to this handle
    ssize_t
    inline __attribute__((always_inline))
    read(void *out, uintptr_t address, size_t size) {
        static struct iovec local[1];
        static struct iovec remote[1];
        local[0].iov_base = out;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        return process_vm_readv(pid, local, 1, remote, 1, 0);
    }

    ssize_t
    inline __attribute__((always_inline))
    write(uintptr_t address, void *in, size_t size) {
        static struct iovec local[1];
        static struct iovec remote[1];
        local[0].iov_base = in;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        return process_vm_writev(pid, local, 1, remote, 1, 0);
    }

/* ptrace peek buffer, used by peekdata() as a mirror of the process memory.
 * Max size is the maximum allowed rounded VLT scan length, aka UINT16_MAX,
 * plus a `PEEKDATA_CHUNK`, to store a full extra chunk for maneuverability */
    // FIXME[critical]: not returning negative values
    /*     1048576 allocated
     * read, read, PEEKDATA_CHUNK: 1048, MAX_this->SIZE: 6_MiB+PEEKDATA_CHUNK
     * 1:  2480510
     * 2:  2160006 2480510-2160006=320504=0x4E3F8
     * 3:  1963873 2160006-1963873=196133=0x2FE25
     * cached, read, PEEKDATA_CHUNK: 1048, MAX_this->SIZE: 6_MiB+PEEKDATA_CHUNK
     * 1:  2492798
     * 2:  2160006 2492798-2160006=332792=0x513F8
     * 3:  1963873 2160006-1963873=196133=0x2FE25
     * read, cached, PEEKDATA_CHUNK: 1048, MAX_this->SIZE: 6_MiB+PEEKDATA_CHUNK
     * 1:  2480510
     * 2:  2160013 2480510-2160013=320497=0x4E3F1
     * 3:  1963885 2160013-1963885=196128=0x2FE20
     * cached, cached, PEEKDATA_CHUNK: 1048, MAX_this->SIZE: 6_MiB+PEEKDATA_CHUNK
     * 1:  2492798
     * 2:  2171333 2492798-2171333=321465=0x4E7B9
     * 3:  1974076 2171333-1974076=197257=0x30289
     *
     */

    class Cached
    {
        Handle& _parent;
        const static size_t PEEKDATA_CHUNK = 2_KiB;
        const static size_t MAX_PEEKBUF_SIZE = 64_KiB + PEEKDATA_CHUNK;
    public:
        uintptr_t base = 0; // base address of cached region
        std::vector<uint8_t> cache;
        size_t size; // amount of valid memory stored (in bytes)

        explicit Cached(Handle& parent) : _parent(parent) {
            cache.resize(MAX_PEEKBUF_SIZE);
        }

        ssize_t
        inline
//        __attribute__((always_inline))
//        __attribute__((noinline))
        read(void *out, uintptr_t address, size_t size)
        {
            unsigned int i;
            uintptr_t missing_bytes;

            assert(cache.size() <= MAX_PEEKBUF_SIZE);

            /* check if we have a full cache hit */
            if (this->base &&
                address >= this->base &&
                (unsigned long) (address + size - this->base) <= cache.size())
            {
                out = (mem64_t*)&this->cache[address - this->base];
                return this->base - address + cache.size();
            }
            else if (this->base &&
                     address >= this->base &&
                     (unsigned long) (address - this->base) < cache.size())
            {
                assert(cache.size() != 0);

                /* partial hit, we have some of the data but not all, so remove old entries - shift the frame by as far as is necessary */
                missing_bytes = (address + size) - (this->base + cache.size());
                /* round up to the nearest PEEKDATA_CHUNK multiple, that is what could
                 * potentially be read and we have to fit it all */
                missing_bytes = PEEKDATA_CHUNK * (1 + (missing_bytes-1) / PEEKDATA_CHUNK);

                /* head shift if necessary */
                if (cache.size() + missing_bytes > MAX_PEEKBUF_SIZE)
                {
                    uintptr_t shift_size = address - this->base;
                    shift_size = PEEKDATA_CHUNK * (shift_size / PEEKDATA_CHUNK);

                    memmove(&this->cache[0], &this->cache[shift_size], cache.size()-shift_size);

                    cache.resize(cache.size()-shift_size);
                    this->base += shift_size;
                }
            }
            else {
                /* cache miss, invalidate the cache */
                missing_bytes = size;
                cache.resize(0);
                this->base = address;
            }

            /* we need to retrieve memory to complete the request */
            for (i = 0; i < missing_bytes; i += PEEKDATA_CHUNK)
            {
                const uintptr_t target_address = this->base + cache.size();
                ssize_t len = _parent.read(&this->cache[cache.size()], target_address, PEEKDATA_CHUNK);

                /* check if the read succeeded */
                if (UNLIKELY(len < PEEKDATA_CHUNK)) {
                    if (len == 0) {
                        /* hard failure to retrieve memory */
                        return 0;
                    }
                    /* go ahead with the partial read and stop the gathering process */
                    cache.resize(cache.size()+len);
                    break;
                }
                /* otherwise, the read worked */
                cache.resize(cache.size()+PEEKDATA_CHUNK);
            }

            /* return result to caller */
            out = (mem64_t*)&this->cache[address - this->base];
            return this->base - address + cache.size();
        }

    } cached;

    /// Modules
    // fixme incredibly large function
    void
    update_regions() {
        using namespace std;
        regions.clear();
        regions_ignored.clear();
        ifstream maps("/proc/" + to_string(pid) + "/maps");
        string line;
        while (getline(maps, line)) {
            istringstream iss(line);
            string memorySpace, permissions, offset, device, inode;
            if (iss >> memorySpace >> permissions >> offset >> device >> inode) {
                string pathname;

                for (size_t ls = 0, i = 0; i < line.length(); i++) {
                    if (line.substr(i, 1) == " ") {
                        ls++;

                        if (ls == 5) {
                            size_t begin = line.substr(i, line.size()).find_first_not_of(' ');

                            if (begin != string::npos)
                                pathname = line.substr(begin + i, line.size());
                            else
                                pathname.clear();
                        }
                    }
                }
                
                Cregion region;

                size_t memorySplit = memorySpace.find_first_of('-');
                size_t deviceSplit = device.find_first_of(':');
                uintptr_t rend;

                stringstream ss;

                if (memorySplit != string::npos) {
                    ss << hex << memorySpace.substr(0, memorySplit);
                    ss >> region.address;
                    ss.clear();
                    ss << hex << memorySpace.substr(memorySplit + 1, memorySpace.size());
                    ss >> rend;
                    region.size = (region.address < rend) ? (rend - region.address) : 0;
                    ss.clear();
                }

                if (deviceSplit != string::npos) {
                    ss << hex << device.substr(0, deviceSplit);
                    ss >> region.deviceMajor;
                    ss.clear();
                    ss << hex << device.substr(deviceSplit + 1, device.size());
                    ss >> region.deviceMinor;
                    ss.clear();
                }

                ss << hex << offset;
                ss >> region.offset;
                ss.clear();
                ss << inode;
                ss >> region.inodeFileNumber;

                region.flags = 0;
                region.flags |= (permissions[0] == 'r') ? readable : 0;
                region.flags |= (permissions[1] == 'w') ? writable : 0;
                region.flags |= (permissions[2] == 'x') ? executable : 0;
                region.flags |= (permissions[3] == '-') ? shared : 0;

                if (!pathname.empty()) {
                    region.pathname = pathname;

                    size_t fileNameSplit = pathname.find_last_of('/');

                    if (fileNameSplit != string::npos) {
                        region.filename = pathname.substr(fileNameSplit + 1, pathname.size());
                    }
                }

                if (region.flags & (readable | writable) && !(region.flags & shared) && region.size > 0)
                    regions.push_back(region);
                else
                    regions_ignored.push_back(region);
            }
        }
    }

    Cregion *
    get_region_by_name(const std::string& region_name) {
        using namespace std;
        for (Cregion& region : regions)
            if (region.flags & executable && region.filename == region_name)
                return &region;
        return nullptr;
    }

    Cregion *
    get_region_of_address(uintptr_t address) {
        using namespace std;
        static Cregion *last_region;
        if (last_region && last_region->address <= address && address < last_region->address + last_region->size) {
            clog << "returning last region " << endl;
            return last_region;
        }
        static size_t first, last, mid;
        first = 0;
        last = regions.size();
        while (first < last) {
            mid = (first + last) / 2;
            if (address < regions[mid].address)
                last = mid - 1;
            else if (address >= regions[mid].address + regions[mid].size)
                first = mid + 1;
            else {
                last_region = &regions[mid];
                return last_region;
            }
        }
        return nullptr;
    }

    // TODO: may be moved to scanner.hh
    bool
    find_pattern(uintptr_t *out, Cregion *region, const char *pattern, const char *mask) {
        char buffer[0x1000];

        uintptr_t len = strlen(mask);
        uintptr_t chunksize = sizeof(buffer);
        uintptr_t totalsize = region->size;
        uintptr_t chunknum = 0;

        while (totalsize) {
            uintptr_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
            uintptr_t readaddr = region->address + (chunksize * chunknum);
            bzero(buffer, chunksize);

            if (this->read(buffer, readaddr, readsize)) {
                for (uintptr_t b = 0; b < readsize; b++) {
                    uintptr_t matches = 0;

                    // если данные совпадают или пропустить
                    while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
                        matches++;

                        if (matches == len) {
                            *out = readaddr + b;
                            return true;
                        }
                    }
                }
            }

            totalsize -= readsize;
            chunknum++;
        }
        return false;
    }

    uintptr_t
    get_call_address(uintptr_t address) {
        static uintptr_t code;
        if (read(&code, address + 1, sizeof(code)) == sizeof(code))
            return code + address + 5;
        return 0;
    }
};


/*bool //todo короче можно вместо стрима закинуть, например, вектор со стрингами
Handle::findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, size_t point_size, std::ostream *ss) 
{
    void *buffer;
    if (ss) *ss<<"["<<offsets[0]<<"] -> ";
    if (!this->read(&buffer, (void *)(address), point_size)) return false;
    if (ss) *ss<<""<<buffer<<endl;
    for(uint64_t i = 0; i < offsets.size()-1; i++) {
        if (ss) *ss<<"["<<buffer<<" + "<<offsets[i]<<"] -> ";
        if (!this->read(&buffer, buffer + offsets[i], point_size)) return false;
        if (ss) *ss<<""<<buffer<<endl;
    }
    if (ss) *ss<<buffer<<" + "<<offsets[offsets.size()-1]<<" = "<<buffer+offsets[offsets.size()-1]<<" -> ";
    if (!this->read(out, buffer + offsets[offsets.size() - 1], size)) return false;

    return true;
}

size_t
Handle::findPattern(vector<uintptr_t> *out, Cregion *region, const char *pattern, const char *mask)
{
    char buffer[0x1000];

    size_t len = strlen(mask);
    size_t chunksize = sizeof(buffer);
    size_t totalsize = region->end - region->address;
    size_t chunknum = 0;
    size_t found = 0;

    while (totalsize) {
        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
        size_t readaddr = region->address + (chunksize * chunknum);
        bzero(buffer, chunksize);

        if (this->read(buffer, (void *) readaddr, readsize)) {
            for(size_t b = 0; b < readsize; b++) {
                size_t matches = 0;

                // если данные совпадают или пропустить
                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
                    matches++;

                    if (matches == len) {
                        found++;
                        out->push_back((uintptr_t) (readaddr + b));
                    }
                }
            }
        }

        totalsize -= readsize;
        chunknum++;
    }
    return found;
}

size_t
Handle::scan_exact(vector<Entry> *out, 
                   const Cregion *region,
                   vector<Entry> entries, 
                   size_t increment)
{
    byte buffer[0x1000];

    size_t chunksize = sizeof(buffer);
    size_t totalsize = region->end - region->address;
    size_t chunknum = 0;
    size_t found = 0;
    // TODO[HIGH] научить не добавлять, если предыдущий (собсна, наибольший) уже есть 
    while (totalsize) {
        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
        uintptr_t readaddr = region->address + (chunksize * chunknum);
        bzero(buffer, chunksize);

        if (this->read(buffer, (void *) readaddr, readsize)) {    // read into buffer
            for(uintptr_t b = 0; b < readsize; b += increment) {  // for each addr inside buffer
                for(int k = 0; k < entries.size(); k++) {         // for each entry
                    size_t matches = 0;
                    while (buffer[b + matches] == entries[k].value.bytes[matches]) {  // находим адрес
                        matches++;

                        if (matches == SIZEOF_FLAG(entries[k].flags)) {
                            found++;
                            out->emplace_back(entries[k].flags, (uintptr_t)(readaddr + b), region, entries[k].value.bytes);
                            //todo мне кажется, что нужно всё-таки добавить плавующую точку, посмотрим, как сделаю scan_reset
                            goto sorry_for_goto;
                        }
                    }
                }
                sorry_for_goto: ;
            }
        }

        totalsize -= readsize;
        chunknum++;
    }
    return found; //size of pushed back values
}*/

} //namespace RE

#endif //RE_CORE_HH
