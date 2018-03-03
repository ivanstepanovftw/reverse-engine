//
// Created by root on 12.02.18.
//

#ifndef HACKME_SDK_HH
#define HACKME_SDK_HH

#define HEX(a) ("0x")<<hex<<(a)<<dec

#include <sys/uio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>


enum ErrorCode { //todo #54 error codes and fn like dlError();
//    catch (const Glib::Error &e) {
//        std::cerr << e.what() << std::endl;
//        return 1;
//    }
};

using namespace std;

std::vector<std::string> split(const std::string &text, const std::string &delims, uint64_t d = UINT64_MAX);

std::string execute(std::string cmd);

std::vector<std::vector<std::string>> getProcesses();

//typedef uint8_t byte;

typedef struct {
    // Memory
    uintptr_t start;
    uintptr_t end;

    // Permissions
    bool readable;
    bool writable;
    bool executable;
    bool shared;

    // File data
    uintptr_t offset;
    unsigned char deviceMajor;
    unsigned char deviceMinor;
    unsigned long inodeFileNumber;
    std::string pathname;
    std::string filename;
} region_t;



/** Scanner implementation */
enum ScanType {
    NUMBER,
    AOB,
    STRING,
    GROUPED,
};

enum ValueType {
    ALL,
    CHAR,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
};

class AddressEntry {
public:
    ValueType type;
    union {
        byte a[sizeof(uintptr_t)];
        uintptr_t n;
    } address;
    const region_t *region;
    
    AddressEntry(ValueType type, uintptr_t address, const region_t *region) // NOLINT
    {
        this->type = type;
        this->address.n = address;
        this->region = region;
    }
};

class PatternEntry {
public:
    ValueType type;
    vector<byte> bytes;
    
    PatternEntry(ValueType type, const vector<byte> &bytes)
    {
        this->type = type;
        this->bytes = bytes;
    }
};


class Handle {
public:
/// Variables
    pid_t pid;
    std::string pidStr;
    std::string title;
    std::vector<region_t> regions;

/// Costructors / Destrictors
    explicit Handle(pid_t pid) {
        stringstream buffer;
        buffer << pid;
        this->pid = pid;
        pidStr = buffer.str();
    }

    explicit Handle(const std::string &title) {
        if (title.empty()) {
            pid = -1;
            pidStr.clear();
        }
        struct dirent *dire;
        DIR *dir = opendir("/proc/");
        if (dir) {
            while ((dire = readdir(dir)) != nullptr) {
                if (dire->d_type != DT_DIR)
                    continue;

                string mapsPath = ("/proc/" + string(dire->d_name) + "/maps");
                if (access(mapsPath.c_str(), F_OK) == -1)
                    continue;

                static string nn = dire->d_name;
                // Check to see if the string is numeric (no negatives or dec allowed, which makes this function usable)
                if (strspn(nn.c_str(), "0123456789") != nn.size()) {
                    this->pid = -1;
                    pidStr.clear();
                } else {
                    istringstream buffer(dire->d_name);
                    pidStr = dire->d_name;
                    buffer >> pid;
                }

                if (!isValid() || !isRunning())
                    continue;

                string procPath = getPath();
                if (procPath.empty())
                    continue;

                size_t namePos = procPath.find_last_of('/');
                if (namePos == -1)
                    continue;

                string exeName = procPath.substr(namePos + 1);
                if (exeName == title) {
                    closedir(dir);
                    this->title = title;
                    return; //success
                }
            }
            closedir(dir);
        }
        pid = -1;
        pidStr.clear();
    }

/// Functions
    // Helpful functions
    std::string getSymbolicLinkTarget(const std::string &target) {
        char buf[PATH_MAX];

        ssize_t len = ::readlink(target.c_str(), buf, sizeof(buf) - 1);

        if (len != -1) {
            buf[len] = 0;
            return string(buf);
        }

        return string();
    }

    std::string getPath() {
        return getSymbolicLinkTarget(("/proc/" + pidStr + "/exe"));
    }

    std::string getWorkingDirectory() {
        return getSymbolicLinkTarget(("/proc/" + pidStr + "/cwd"));
    }

    // Checking
    bool isValid() {
        return pid != -1;
    }

    bool isRunning() {
        if (!isValid())
            return false;

        struct stat sts{};
        return !(stat(("/proc/" + pidStr).c_str(), &sts) == -1&&errno == ENOENT);
    }

    // Read_from/write_to this handle
    bool inline read(void *out, void *address, size_t size) {
        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = out;
        local[0].iov_len = size;
        remote[0].iov_base = address;
        remote[0].iov_len = size;

        return (process_vm_readv(pid, local, 1, remote, 1, 0) == size);
    }

    bool inline write(void *address, void *buffer, size_t size) {
        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = buffer;
        local[0].iov_len = size;
        remote[0].iov_base = address;
        remote[0].iov_len = size;

        return (process_vm_writev(pid, local, 1, remote, 1, 0) == size);
    }

    // Modules
    void updateRegions() {
        regions.clear();
        ifstream maps("/proc/" + pidStr + "/maps");
        string line;
        while(getline(maps, line)) {
            istringstream iss(line);
            string memorySpace, permissions, offset, device, inode;
            if (iss>>memorySpace>>permissions>>offset>>device>>inode) {
                string pathname;

                for(size_t ls = 0, i = 0; i < line.length(); i++) {
                    if (line.substr(i, 1) == " ") {
                        ls++;

                        if (ls == 5) {
                            size_t begin = line.substr(i, line.size()).find_first_not_of(' ');

                            if (begin != -1)
                                pathname = line.substr(begin + i, line.size());
                            else
                                pathname.clear();
                        }
                    }
                }

                region_t region;

                size_t memorySplit = memorySpace.find_first_of('-');
                size_t deviceSplit = device.find_first_of(':');

                stringstream ss;

                if (memorySplit != -1) {
                    ss<<hex<<memorySpace.substr(0, memorySplit);
                    ss>>region.start;
                    ss.clear();
                    ss<<hex<<memorySpace.substr(memorySplit + 1, memorySpace.size());
                    ss>>region.end;
                    ss.clear();
                }

                if (deviceSplit != -1) {
                    ss<<hex<<device.substr(0, deviceSplit);
                    ss>>region.deviceMajor;
                    ss.clear();
                    ss<<hex<<device.substr(deviceSplit + 1, device.size());
                    ss>>region.deviceMinor;
                    ss.clear();
                }

                ss<<hex<<offset;
                ss>>region.offset;
                ss.clear();
                ss<<inode;
                ss>>region.inodeFileNumber;

                region.readable = (permissions[0] == 'r');
                region.writable = (permissions[1] == 'w');
                region.executable = (permissions[2] == 'x');
                region.shared = (permissions[3] != '-');

                if (!pathname.empty()) {
                    region.pathname = pathname;

                    size_t fileNameSplit = pathname.find_last_of('/');

                    if (fileNameSplit != -1) {
                        region.filename = pathname.substr(fileNameSplit + 1, pathname.size());
                    }
                }

                regions.push_back(region);
            }
        }
    }

    region_t *getRegion(const std::string &region_name = "") {
        string filename = region_name;
        if (region_name.empty())
            filename = this->title;
        for (region_t &region : regions) {
            if (region.filename == filename && region.executable) {
                return &region;
            }
        }
        return nullptr;
    }

    region_t *getModuleOfAddress(void *address) {
        for(size_t i = 0; i < regions.size(); i++) {
            if (regions[i].start > (unsigned long) address && (regions[i].start + regions[i].end) <= (unsigned long) address) {
                return &regions[i];
            }
        }

        return nullptr;
    }

    bool getCallAddress(uintptr_t *out, void *address) {
        unsigned long code = 0;

        if (read((char *) address + 1, &code, sizeof(unsigned int))) {
            *out = code + (unsigned long) address + 5;
            return true;
        }

        return false;
    }

    bool findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, size_t point_size = sizeof(uintptr_t),
                     std::ostream *ss = nullptr) {
        void *buffer;
        if (ss) *ss<<"["<<offsets[0]<<"] -> ";
        if (!this->read(&buffer, (void *)(address), point_size)) return false;
        if (ss) *ss<<""<<buffer<<std::endl;
        for(uint64_t i = 0; i < offsets.size()-1; i++) {
            if (ss) *ss<<"["<<buffer<<" + "<<offsets[i]<<"] -> ";
            if (!this->read(&buffer, buffer + offsets[i], point_size)) return false;
            if (ss) *ss<<""<<buffer<<std::endl;
        }
        if (ss) *ss<<buffer<<" + "<<offsets[offsets.size()-1]<<" = "<<buffer+offsets[offsets.size()-1]<<" -> ";
        if (!this->read(out, buffer + offsets[offsets.size() - 1], size)) return false;

        return true;
    }


    bool findPattern(uintptr_t *out, region_t *region, const char *pattern, const char *mask) {
        char buffer[0x1000];

        size_t len = strlen(mask);
        size_t chunksize = sizeof(buffer);
        size_t totalsize = region->end - region->start;
        size_t chunknum = 0;

        while (totalsize) {
            size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
            size_t readaddr = region->start + (chunksize * chunknum);
            bzero(buffer, chunksize);

            if (this->read(buffer, (void *) readaddr, readsize)) {
                for(size_t b = 0; b < readsize; b++) {
                    size_t matches = 0;

                    // если данные совпадают или пропустить
                    while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
                        matches++;

                        if (matches == len) {
//                            printf("Debug Output:\n");
//                            for (int i = 0; i < readsize-b; i++) {
//                                if (i != 0 && i % 8 == 0) {
//                                    printf("\n");
//                                }
//                                printf("%02x ",(unsigned char)buffer[b+i]);
//                            }
//                            printf("\n");
                            *out = (uintptr_t) (readaddr + b);
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
    
    size_t findPattern(vector<uintptr_t> *out, region_t *region, const char *pattern, const char *mask) {
        char buffer[0x1000];
        
        size_t len = strlen(mask);
        size_t chunksize = sizeof(buffer);
        size_t totalsize = region->end - region->start;
        size_t chunknum = 0;
        size_t found = 0;
        
        while (totalsize) {
            size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
            size_t readaddr = region->start + (chunksize * chunknum);
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
    
    //fixme 12345 implement "Value between", "Other than value" and "Unknown initial value"
    //fixme speedup it https://stackoverflow.com/questions/3664272/is-stdvector-so-much-slower-than-plain-arrays
    size_t scan_exact(vector<AddressEntry> *out,
                      const region_t *region,
                      vector<PatternEntry> patterns,  //attempt to implement Number scanner
                      const size_t increment = 1)     //attempt to implement "Fast scan" (он же так работает?)
    {
        byte buffer[0x1000];
        
        size_t chunksize = sizeof(buffer);
        size_t totalsize = region->end - region->start;
        size_t chunknum = 0;
        size_t found = 0;
        
        while (totalsize) {
            size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
            size_t readaddr = region->start + (chunksize * chunknum);
            bzero(buffer, chunksize);
            
            if (this->read(buffer, (void *) readaddr, readsize)) {              //читаем в буфер
                for(size_t b = 0; b < readsize; b += increment) {               //обрабатываем буфер
                    for(const auto &pattern : patterns) {                       //для каждого шаблона
                        size_t matches = 0;
                        while (buffer[b + matches] == pattern.bytes[matches]) { //находим адрес
                            matches++;
                            
                            if (matches == pattern.bytes.size()) {
                                found++;
                                out->emplace_back(pattern.type, (uintptr_t)(readaddr + b), region);
                            }
                        }
                    }
                }
            }
            
            totalsize -= readsize;
            chunknum++;
        }
        return found;
    }
};


class HandleScanner {
public:
    Handle *handle;
    ScanType scan_type;
    
    ValueType value_type;
    size_t increment;
    
    double progress = 0;
    vector<AddressEntry> output;
    
    explicit HandleScanner(Handle *handle,
                           ScanType scan) 
            : handle(handle)
            , scan_type(scan)
    { }
    
    
    void setup_number_scan(ValueType valueType,
                           size_t increment,
                           int round = 0)
    {
        this->value_type = valueType;
        this->increment = increment;
    }
    
    
    void first_scan(const string &pattern)
    {
        output.clear();
        // using boost because http://www.boost.org/doc/libs/1_57_0/doc/html/boost_lexical_cast/performance.html
        if (scan_type == ScanType::NUMBER) { //fixme 12345
            vector<PatternEntry> patterns;
            //fixme find a way to simplify this code
            if (value_type == ValueType::ALL || value_type == ValueType::CHAR) {
                try {
                    auto b = boost::lexical_cast<int8_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::CHAR, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::SHORT) {
                try {
                    auto b = boost::lexical_cast<int16_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::SHORT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::INT) {
                try {
                    auto b = boost::lexical_cast<int32_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::INT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::LONG) {
                try {
                    auto b = boost::lexical_cast<int64_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::LONG, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::FLOAT) {
                try {
                    auto b = boost::lexical_cast<float>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::FLOAT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::DOUBLE) {
                try {
                    auto b = boost::lexical_cast<double>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::DOUBLE, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            
            /// Scanning
            if (!patterns.empty()) {
                for(size_t i = 0; i < patterns.size(); i++)
                    printf("patterns[%li]: %x %x %x %x\n", i, 
                           patterns[i].bytes[0], patterns[i].bytes[1],
                           patterns[i].bytes[2], patterns[i].bytes[3]);
                
                for(int i = 0; i < handle->regions.size(); i++) {
                    region_t region = handle->regions[i];
                    if (region.writable) {
                        handle->scan_exact(&output, &region, patterns, increment);
                    }
                    progress = (i+1)/handle->regions.size();
                }
            }
        }
        else if (scan_type == ScanType::AOB) {
            
        }
        else if (scan_type == ScanType::STRING) {
            
        }
        else if (scan_type == ScanType::GROUPED) {
            
        }
    }
    
    void next_scan(string pattern)
    {
        
    }
    
    void reset()
    {
        output.clear();
    }
    
};



bool worldToScreen(const float *matrix, std::vector<float> *from, std::vector<float> *to);

void *GetModuleHandleSafe(const char *);
void *GetProcAddress(void *, const char*);

#endif //HACKME_SDK_HH



































