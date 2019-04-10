#include <reverseengine/core.hh>


RE::region *
RE::phandler_i::get_region_by_name(const std::string& region_name) {
    for (region& region : regions)
        if ((region.flags & region_mode_t::writable) == 0
        && region.offset == 0
        && region.filename == region_name)
            return &region;
    return nullptr;
}


RE::region *
RE::phandler_i::get_region_of_address(uintptr_t address) const {
    size_t first, last, mid;
    first = 0;
    last = regions.size();
    while (first <= last) {
        mid = (first + last) / 2;
        if (address < regions[mid].address) {
            last = mid - 1;
        } else if (address >= regions[mid].address + regions[mid].size) {
            first = mid + 1;
        } else {
            return (RE::region *)&regions[mid];
        }
    }
    return nullptr;
}

uintptr_t
RE::phandler_i::get_call_address(uintptr_t address) const {
    uint64_t code = 0;
    if (read(address + 1, &code, sizeof(uint32_t)) == sizeof(uint32_t))
        return code + address + 5;
    return 0;
}

uintptr_t
RE::phandler_i::get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) const {
    uint64_t code = 0;
    if (read(address + offset, &code, sizeof(uint32_t)) == sizeof(uint32_t)) {
        return address + code + size;
    }
    return 0;
}


RE::phandler_memory::phandler_memory(const RE::phandler_file& rhs) {
    regions_on_map.clear();
    for (const RE::region& region : rhs.regions) {
        char* map = new char[region.size];
        regions_on_map.emplace_back(map);
        ssize_t copied = rhs.read(region.address, map, region.size);
    }
}


RE::phandler_pid::phandler_pid(pid_t pid) noexcept {
    m_pid = pid;
    try {
        m_cmdline = get_executable().filename();
    } catch (const sfs::filesystem_error &e) {
        /* no such PID running */
    }
    if (m_cmdline.empty()) {
        m_pid = 0;
        m_cmdline = "";
        return;
    }
    update_regions();
}

RE::phandler_pid::phandler_pid(const std::string& title) {
    m_pid = 0;
    if (title.empty())
        return;

    for(auto& p: sfs::directory_iterator("/proc")) {
        if (p.path().filename().string().find_first_not_of("0123456789") != std::string::npos)
            /* if filename is not numeric */
            continue;
        if (!sfs::is_directory(p))
            continue;
        if (!sfs::exists(p / "maps"))
            continue;
        if (!sfs::exists(p / "exe"))
            continue;
        if (!sfs::exists(p / "cmdline"))
            continue;
        std::istringstream ss(p.path().filename().string());
        ss >> m_pid;
        m_cmdline = get_cmdline();

        std::regex rgx(title);
        std::smatch match;
        if (std::regex_search(m_cmdline, match, rgx)) {
            update_regions();
            return;
        }
    }
    m_pid = 0;
    m_cmdline = "";
}