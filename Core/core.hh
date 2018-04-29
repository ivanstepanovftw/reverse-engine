#ifndef RE_CORE_HH
#define RE_CORE_HH

#include <sys/uio.h>
#include <iostream>
#include <experimental/filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include "value.hh"
#include "external.hh"


#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#undef	HEX
#define HEX(s) std::hex<<std::showbase<<(s)<<std::dec


using namespace std;
namespace fs = std::experimental::filesystem;


class Handle
{
public:
    /// Variables
    pid_t pid;
    string title;
    vector<region_t> regions_ignored;
    vector<region_t> regions;
    
    explicit Handle(pid_t pid)
    {
        stringstream buffer;
        buffer<<pid;
        this->pid = pid;
    }
    
    explicit Handle(const string& title)
    {
        if (title.empty()) {
            pid = -1;
            return;
        }
        
        struct dirent *dire;
        DIR *dir = opendir("/proc/");
        if (!dir) {
            pid = -1;
            return;
        }
        while ((dire = readdir(dir)) != NULL) {
            if (dire->d_type != DT_DIR)
                continue;
            const string& nn(dire->d_name);
            
            string mapsPath = "/proc/" + nn + "/maps";
            if (access(mapsPath.c_str(), F_OK) == -1)
                continue;
            
            /// Check to see if the string is numeric (no negatives or dec allowed, which makes this function usable)
            if (strspn(nn.c_str(), "0123456789") == nn.size()) {
                istringstream buffer(nn);
                buffer>>pid;
            } else {
                pid = -1;
            }
            
            if (!is_valid() || !is_running())
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
                break;
            }
        }
        closedir(dir);
    }
    
    /// Functions
    string get_symbolic_link_target(const string& target)
    {
        char buf[PATH_MAX];
        ssize_t len = ::readlink(target.c_str(), buf, sizeof(buf) - 1);
        if (len == -1)
            return ""s;
        buf[len] = '\0';
        return string(buf);
    }
    
    string get_path()
    {
        return get_symbolic_link_target("/proc/" + to_string(pid) + "/exe");
    }
    
    string get_working_directory()
    {
        return get_symbolic_link_target("/proc/" + to_string(pid) + "/cwd");
    }
    
    /// Checking
    bool is_valid()
    {
        return pid != -1;
    }
    
    bool is_running()
    {
        errno = 0;
        return !(stat(("/proc/" + to_string(pid)).c_str(), &sts) == -1 && errno == ENOENT);
    }
    
    bool is_good()
    {
        return is_valid() && is_running();
    }
    
    /// Read_from/write_to this handle
    ssize_t read(void *out, uintptr_t address, size_t size)
    {
        local[0].iov_base = out;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        
        return process_vm_readv(pid, local, 1, remote, 1, 0);
    }
    
    ssize_t write(uintptr_t address, void *in, size_t size)
    {
        local[0].iov_base = in;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        
        return process_vm_writev(pid, local, 1, remote, 1, 0);
    }
    
    /// Modules
    // fixme incredibly large function
    void update_regions()
    {
        regions_ignored.clear();
        regions.clear();
        ifstream maps("/proc/" + to_string(pid) + "/maps");
        string line;
        while (getline(maps, line)) {
            istringstream iss(line);
            string memorySpace, permissions, offset, device, inode;
            if (iss>>memorySpace>>permissions>>offset>>device>>inode) {
                string pathname;
                
                for(size_t ls = 0, i = 0; i < line.length(); i++) {
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
                
                region_t region;
                
                size_t memorySplit = memorySpace.find_first_of('-');
                size_t deviceSplit = device.find_first_of(':');
                
                stringstream ss;
                
                if (memorySplit != string::npos) {
                    ss<<hex<<memorySpace.substr(0, memorySplit);
                    ss>>region.address;
                    ss.clear();
                    ss<<hex<<memorySpace.substr(memorySplit + 1, memorySpace.size());
                    uintptr_t end;
                    ss>>end;
                    region.size = (region.address < end) ? (end - region.address) : 0;
                    ss.clear();
                }
                
                if (deviceSplit != string::npos) {
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
                
                region.flags = 0;
                region.flags |= (permissions[0] == 'r') ? readable : byte(0);
                region.flags |= (permissions[1] == 'w') ? writable : byte(0);
                region.flags |= (permissions[2] == 'x') ? executable : byte(0);
                region.flags |= (permissions[3] == '-') ? shared : byte(0);
                
                if (!pathname.empty()) {
                    region.pathname = pathname;
                    
                    size_t fileNameSplit = pathname.find_last_of('/');
                    
                    if (fileNameSplit != string::npos) {
                        region.filename = pathname.substr(fileNameSplit + 1, pathname.size());
                    }
                }
                
                if (region.flags & writable && region.flags & readable && region.size > 0)
                    regions.push_back(region);
                else
                    regions_ignored.push_back(region);
            }
        }
    }
    
    region_t *get_region_by_name(const string& region_name = "")
    {
        string filename = region_name;
        if (region_name.empty())
            filename = title;
        for(region_t& region : regions) {
            if (region.filename == filename && region.flags & executable) {
                return &region;
            }
        }
        return nullptr;
    }
    
    const region_t *get_region_of_address(uintptr_t address)
    {
        for(const region_t& region : regions)
            if (region.address <= address && address < region.address + region.size)
                return &region;
        return nullptr;
    }
    
    const region_t *get_region_by_address(uintptr_t address)
    {
        for(const region_t& region : regions)
            if (region.address == address)
                return &region;
        return nullptr;
    }
    
    bool get_call_address(uintptr_t *out, uintptr_t address)
    {
        uintptr_t code = 0;
        
        if (read(&code, address + 1, sizeof(code))) {
            *out = code + address + 5;
            return true;
        }
        
        return false;
    }

protected:
    struct iovec local[1];  // read, write
    struct iovec remote[1];
    struct stat sts{};      // is_running
};

#endif //RE_CORE_HH
