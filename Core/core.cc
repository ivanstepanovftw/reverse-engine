//
// Created by root on 16.02.18.
//

#include "core.hh"

using namespace std;

Handle::Handle(pid_t pid) 
{
    stringstream buffer;
    buffer << pid;
    this->pid = pid;
    pidStr = buffer.str();
}

Handle::Handle(const std::string &title) 
{
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
            if (namePos == string::npos)
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

std::string
Handle::getSymbolicLinkTarget(const std::string &target) 
{
    char buf[PATH_MAX];
    
    ssize_t len = ::readlink(target.c_str(), buf, sizeof(buf) - 1);
    
    if (len != -1) {
        buf[len] = 0;
        return string(buf);
    }
    
    return string();
}

std::string
Handle::getPath() 
{
    return getSymbolicLinkTarget(("/proc/" + pidStr + "/exe"));
}

std::string
Handle::getWorkingDirectory() 
{
    return getSymbolicLinkTarget(("/proc/" + pidStr + "/cwd"));
}

bool
Handle::isValid() 
{
    return pid != -1;
}

bool
Handle::isRunning() 
{
    if (!isValid())
        return false;
    
    struct stat sts{};
    return !(stat(("/proc/" + pidStr).c_str(), &sts) == -1&&errno == ENOENT);
}

bool
Handle::read(void *out, uintptr_t address, ssize_t size) 
{
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = out;
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = size;
    
    return (process_vm_readv(pid, local, 1, remote, 1, 0) == size);
}

bool
Handle::write(uintptr_t address, void *buffer, ssize_t size) 
{
    struct iovec local[1];
    struct iovec remote[1];
    
    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = size;
    
    return (process_vm_writev(pid, local, 1, remote, 1, 0) == size);
}

void
Handle::updateRegions() 
{
    regions_all.clear();
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
                ss>>region.start;
                ss.clear();
                ss<<hex<<memorySpace.substr(memorySplit + 1, memorySpace.size());
                ss>>region.end;
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
            
            region.readable = (permissions[0] == 'r');
            region.writable = (permissions[1] == 'w');
            region.executable = (permissions[2] == 'x');
            region.shared = (permissions[3] != '-');
            
            if (!pathname.empty()) {
                region.pathname = pathname;
                
                size_t fileNameSplit = pathname.find_last_of('/');
                
                if (fileNameSplit != string::npos) {
                    region.filename = pathname.substr(fileNameSplit + 1, pathname.size());
                }
            }
            
            regions_all.push_back(region);
            if (region.writable && region.readable)
                if (region.start < region.end)
                    regions.push_back(region);
        }
    }
}

region_t *
Handle::getRegion(const std::string &region_name) {
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

region_t *
Handle::getRegionOfAddress(uintptr_t address) 
{
    for(region_t &region : regions)
        if (region.start <= address && address < region.end)
            return &region;
    return nullptr;
}

bool
Handle::getCallAddress(uintptr_t *out, uintptr_t address) 
{
    uintptr_t code = 0;
    
    if (read(&code, address + 1, sizeof(code))) {
        *out = code + address + 5;
        return true;
    }
    
    return false;
}

//bool //todo короче можно вместо стрима закинуть, например, вектор со стрингами
//Handle::findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, size_t point_size, std::ostream *ss) 
//{
//    void *buffer;
//    if (ss) *ss<<"["<<offsets[0]<<"] -> ";
//    if (!this->read(&buffer, (void *)(address), point_size)) return false;
//    if (ss) *ss<<""<<buffer<<std::endl;
//    for(uint64_t i = 0; i < offsets.size()-1; i++) {
//        if (ss) *ss<<"["<<buffer<<" + "<<offsets[i]<<"] -> ";
//        if (!this->read(&buffer, buffer + offsets[i], point_size)) return false;
//        if (ss) *ss<<""<<buffer<<std::endl;
//    }
//    if (ss) *ss<<buffer<<" + "<<offsets[offsets.size()-1]<<" = "<<buffer+offsets[offsets.size()-1]<<" -> ";
//    if (!this->read(out, buffer + offsets[offsets.size() - 1], size)) return false;
//    
//    return true;
//}
//
//bool
//Handle::findPattern(uintptr_t *out, region_t *region, const char *pattern, const char *mask) 
//{
//    char buffer[0x1000];
//    
//    size_t len = strlen(mask);
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->start;
//    size_t chunknum = 0;
//    
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        size_t readaddr = region->start + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {
//            for(size_t b = 0; b < readsize; b++) {
//                size_t matches = 0;
//                
//                // если данные совпадают или пропустить
//                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
//                    matches++;
//                    
//                    if (matches == len) {
////                            printf("Debug Output:\data");
////                            for (int i = 0; i < readsize-b; i++) {
////                                if (i != 0 && i % 8 == 0) {
////                                    printf("\data");
////                                }
////                                printf("%02x ",(unsigned char)buffer[b+i]);
////                            }
////                            printf("\data");
//                        *out = (uintptr_t) (readaddr + b);
//                        return true;
//                    }
//                }
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return false;
//}
//
//size_t
//Handle::findPattern(vector<uintptr_t> *out, region_t *region, const char *pattern, const char *mask) 
//{
//    char buffer[0x1000];
//    
//    size_t len = strlen(mask);
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->start;
//    size_t chunknum = 0;
//    size_t found = 0;
//    
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        size_t readaddr = region->start + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {
//            for(size_t b = 0; b < readsize; b++) {
//                size_t matches = 0;
//                
//                // если данные совпадают или пропустить
//                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
//                    matches++;
//                    
//                    if (matches == len) {
//                        found++;
//                        out->push_back((uintptr_t) (readaddr + b));
//                    }
//                }
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return found;
//}
//
//size_t
//Handle::scan_exact(vector<Entry> *out, 
//                   const region_t *region, 
//                   vector<Entry> entries, 
//                   size_t increment)
//{
//    byte buffer[0x1000];
//    
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->start;
//    size_t chunknum = 0;
//    size_t found = 0;
//    // TODO[HIGH] научить не добавлять, если предыдущий (собсна, наибольший) уже есть 
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        uintptr_t readaddr = region->start + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {    // read into buffer
//            for(uintptr_t b = 0; b < readsize; b += increment) {  // for each addr inside buffer
//                for(int k = 0; k < entries.size(); k++) {         // for each entry
//                    size_t matches = 0;
//                    while (buffer[b + matches] == entries[k].value.bytes[matches]) {  // находим адрес
//                        matches++;
//                        
//                        if (matches == SIZEOF_FLAG(entries[k].flags)) {
//                            found++;
//                            out->emplace_back(entries[k].flags, (uintptr_t)(readaddr + b), region, entries[k].value.bytes);
//                            //todo мне кажется, что нужно всё-таки добавить плавующую точку, посмотрим, как сделаю next_scan
//                            goto sorry_for_goto;
//                        }
//                    }
//                }
//                sorry_for_goto: ;
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return found; //size of pushed back values
//}
