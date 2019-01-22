#include <reverseengine/core.hh>

void
RE::Handle::attach(pid_t pid) {
    this->pid = pid;
    try {
        this->title = this->get_path().filename();
    } catch (const sfs::filesystem_error& e) { /* no such PID running */ }
    if (this->title.empty()) {
        this->pid = 0;
        this->title = "";
        return;
    }
}


void
RE::Handle::attach(const std::string& title) {
    pid = 0;
    if (title.empty())
        return;

    // Linux
    try {
        for(auto& p: sfs::directory_iterator(sfs::current_path().root_path()/"proc")) {  // usually path is /proc/**
            if (!sfs::is_directory(p))
                continue;
            if (!sfs::exists(p/"exe"))  // without this we cannot get pid by title (executable)
                continue;
            if (!sfs::exists(p/"maps"))
                continue;
            if (p.path().filename().string().find_first_not_of("0123456789") != std::string::npos)  // check if filename is not numeric
                continue;

            std::istringstream str_pid(p.path().filename().string());
            str_pid >> this->pid;
            if (!this->is_good()) {
                continue;
            }

            // todo[medium]: okay, we got alot of chromium, but which chromium is on top of others? Hint: cat /proc/1234/cmdline
            if (sfs::canonical(p/"exe").filename() == title) {
                this->title = title;
                return;
            }
            this->pid = 0;
        }
    } catch (const sfs::filesystem_error& e) { /* not linux or linux w/o "/proc" folder */ }
}


// fixme[medium]: incredibly large function
void RE::Handle::update_regions() {
    regions.clear();
    regions_ignored.clear();
    std::ifstream maps(sfs::current_path().root_path()/"proc"/std::to_string(pid)/"maps");
    std::string line;
    while (getline(maps, line)) {
        std::istringstream iss(line);
        std::string memorySpace, permissions, offset, device, inode;
        if (iss >> memorySpace >> permissions >> offset >> device >> inode) {
            std::string pathname;

            for (size_t ls = 0, i = 0; i < line.length(); i++) {
                if (line.substr(i, 1) == " ") {
                    ls++;

                    if (ls == 5) {
                        size_t begin = line.substr(i, line.size()).find_first_not_of(' ');

                        if (begin != std::string::npos)
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

            std::stringstream ss;

            if (memorySplit != std::string::npos) {
                ss << std::hex << memorySpace.substr(0, memorySplit);
                ss >> region.address;
                ss.clear();
                ss << std::hex << memorySpace.substr(memorySplit + 1, memorySpace.size());
                ss >> rend;
                region.size = (region.address < rend) ? (rend - region.address) : 0;
                ss.clear();
            }

            if (deviceSplit != std::string::npos) {
                ss << std::hex << device.substr(0, deviceSplit);
                ss >> region.deviceMajor;
                ss.clear();
                ss << std::hex << device.substr(deviceSplit + 1, device.size());
                ss >> region.deviceMinor;
                ss.clear();
            }

            ss << std::hex << offset;
            ss >> region.offset;
            ss.clear();
            ss << inode;
            ss >> region.inodeFileNumber;

            region.flags = region_mode_t::none;
            region.flags |= (permissions[0] == 'r') ? region_mode_t::readable : region_mode_t::none;
            region.flags |= (permissions[1] == 'w') ? region_mode_t::writable : region_mode_t::none;
            region.flags |= (permissions[2] == 'x') ? region_mode_t::executable : region_mode_t::none;
            region.flags |= (permissions[3] == '-') ? region_mode_t::shared : region_mode_t::none;

            if (!pathname.empty()) {
                region.pathname = pathname;

                size_t fileNameSplit = pathname.find_last_of('/');

                if (fileNameSplit != std::string::npos) {
                    region.filename = pathname.substr(fileNameSplit + 1, pathname.size());
                }
            }

            if (region.flags & (region_mode_t::readable | region_mode_t::writable) && !(region.flags & region_mode_t::shared) && region.size > 0)
                regions.push_back(region);
            else
                regions_ignored.push_back(region);
        }
    }
    regions.shrink_to_fit();
    regions_ignored.shrink_to_fit();
}
