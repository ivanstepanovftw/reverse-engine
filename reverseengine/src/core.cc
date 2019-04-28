#include <reverseengine/core.hh>
#include <omp.h>

namespace sfs = std::filesystem;
namespace bio = boost::iostreams;


///
/// RE::IProcess
///

RE::Region *
RE::IProcess::get_region_by_name(const std::string& region_name) {
    for (Region& region : regions)
        if (region.offset == 0
        && (region.flags & region_mode_t::writable) == 0
        && region.file.filename() == region_name)
            return &region;
    return nullptr;
}

RE::Region *
RE::IProcess::get_region_of_address(uintptr_t address) const {
    size_t first, last, mid;
    first = 0;
    last = regions.size();
    while (first < last) {
        mid = (first + last) / 2;
        if (address < regions[mid].address) {
            last = mid - 1;
        } else if (address >= regions[mid].address + regions[mid].size) {
            first = mid + 1;
        } else {
            return (RE::Region *)&regions[mid];
        }
    }
    return nullptr;
}

uintptr_t
RE::IProcess::get_call_address(uintptr_t address) const {
    uint64_t code = 0;
    if (read(address + 1, &code, sizeof(uint32_t)) == sizeof(uint32_t))
        return code + address + 5;
    return 0;
}

uintptr_t
RE::IProcess::get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) const {
    uint64_t code = 0;
    if (read(address + offset, &code, sizeof(uint32_t)) == sizeof(uint32_t)) {
        return address + code + size;
    }
    return 0;
}


///
/// RE::Process
///

RE::Process::Process(pid_t pid) noexcept {
    m_pid = pid;
    try {
        m_cmdline = get_executable().filename();
    } catch (const sfs::filesystem_error &e) {
        /* no such PID running */
    }
    if (m_cmdline.empty()) {
        m_pid = 0;
        m_cmdline.clear();
        return;
    }
    update_regions();
}

RE::Process::Process(const std::string& title) {
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
    m_cmdline.clear();
}

sfs::path
RE::Process::get_executable() {
    /* Ambiguous */
    std::string exe = sfs::path(get_exe());
    for (const Region& region : regions) {
        if (region.file == exe) {
            struct stat sb{};
            errno = 0;
            int s = stat(exe.c_str(), &sb);
            auto ma = major(sb.st_dev);
            auto mi = minor(sb.st_dev);
            if (s == 0 && ma == region.st_device_major && mi == region.st_device_minor && sb.st_ino == region.inode && region.offset == 0)
                return sfs::path(exe);
        }
    }
    throw std::runtime_error("File not found");
}

size_t
RE::Process::read(uintptr_t address, void *out, size_t size) const {
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = out;
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = size;
    return static_cast<size_t>(process_vm_readv(m_pid, local, 1, remote, 1, 0));
}

size_t
RE::Process::write(uintptr_t address, void *in, size_t size) const {
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = in;
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void *>(address);
    remote[0].iov_len = size;
    return static_cast<size_t>(process_vm_writev(m_pid, local, 1, remote, 1, 0));
}

void
RE::Process::update_regions() {
    regions.clear();
    std::ifstream maps(sfs::path("/proc") / std::to_string(get_pid()) / "maps");

    std::string line;
    while (getline(maps, line)) {
        // 00601000-00602000 rw-p 00001000 08:11 3427227      /bin/FAKEMEM
        std::vector<std::string> cols = tokenize(line, " ", 6);
        if (cols.size() < 5)
            continue;
        Region r;
        std::vector<std::string> mem = tokenize(cols[0], "-");
        r.address = strtoumax(mem[0].c_str(), nullptr, 16);
        r.size = strtoumax(mem[1].c_str(), nullptr, 16) - r.address;
        r.flags |= (cols[1][0] == 'r') ? region_mode_t::readable : region_mode_t::none;
        r.flags |= (cols[1][1] == 'w') ? region_mode_t::writable : region_mode_t::none;
        r.flags |= (cols[1][2] == 'x') ? region_mode_t::executable : region_mode_t::none;
        r.flags |= (cols[1][3] == 's') ? region_mode_t::shared : region_mode_t::none;
        r.offset = strtoumax(cols[2].c_str(), nullptr, 16);
        std::vector<std::string> dev = tokenize(cols[3], ":");
        r.st_device_major = strtoumax(dev[0].c_str(), nullptr, 16);
        r.st_device_minor = strtoumax(dev[1].c_str(), nullptr, 16);
        r.inode = strtoumax(cols[4].c_str(), nullptr, 10);
        if (cols.size() > 5)
            r.file = sfs::path(cols[5]);
        regions.push_back(r);
    }
    regions.shrink_to_fit();
}


///
/// RE::IProcessM
///

/** Read value */
size_t
RE::IProcessM::read(uintptr_t address, void *out, size_t size) const {
    using namespace std;
    Region *r = get_region_of_address(address);
    if UNLIKELY(r == nullptr)
        return npos;
    if UNLIKELY(address + size > r->address + r->size) {
        size = r->size + r->address - address;
    }
    memcpy(out, reinterpret_cast<char *>(regions_on_map[r - &regions[0]] + (address - r->address)), size);
    return size;
}

/** Write value */
size_t
RE::IProcessM::write(uintptr_t address, void *in, size_t size) const {
    return static_cast<size_t>(size);
}

bool
RE::IProcessM::is_valid() const {
    return false;
}

void
RE::IProcessM::update_regions() {

}

bool
RE::IProcessM::operator!() {
    return false;
}


///
/// RE::ProcessH
///

RE::ProcessH::ProcessH(const RE::ProcessF& rhs) {
    regions_on_map.clear();
    for (const RE::Region& region : rhs.regions) {
        char* map = new char[region.size];
        regions_on_map.emplace_back(map);
        ssize_t copied = rhs.read(region.address, map, region.size);
    }
}


///
/// RE::ProcessF
///

void
RE::ProcessF::save(const Process& handler, const std::string& path) {
    regions = handler.regions;

    std::ofstream stream(path, std::ios_base::out | std::ios_base::binary);
    boost::archive::binary_oarchive archive(stream, boost::archive::no_header);
    archive << *this;
    stream.flush();

    params.path = path;
    params.flags = bio::mapped_file::mapmode::readwrite;
    mf.open(params);
    if (!mf.is_open())
        throw std::invalid_argument("can not open '" + path + "'");

    size_t bytes_to_save = 0;
    for (const RE::Region& region : handler.regions)
        bytes_to_save += sizeof(region.size) + region.size;
    mf.resize(mf.size() + bytes_to_save);

    char* snapshot = mf.data();
    assert(stream.is_open());
    assert(stream.tellp() != -1);
    snapshot += stream.tellp();

    regions_on_map.clear();
    for(const RE::Region& region : handler.regions) {
        regions_on_map.emplace_back(snapshot);
        ssize_t copied = handler.read(region.address, snapshot, region.size);
        //snapshot += region.size + sizeof(mem64_t::bytes) - 1;
        snapshot += region.size;
    }
}

void
RE::ProcessF::load(const std::string& path) {
    std::ifstream stream(path, std::ios_base::in | std::ios_base::binary);
    std::string m(magic);
    stream.read(&m[0], m.size());

    if (!stream || m != magic) {
        return;
    }
    boost::archive::binary_iarchive archive(stream, boost::archive::no_header);
    archive >> *this;

    params.path = path;
    params.flags = bio::mapped_file::mapmode::readwrite;
    mf.open(params);
    if (!mf.is_open())
        throw std::invalid_argument("can not open '" + path + "'");

    char* snapshot = mf.data();
    assert(stream.is_open());
    assert(stream.tellg() != -1);
    snapshot += stream.tellg();

    regions_on_map.clear();
    for (const RE::Region& region : regions) {
        regions_on_map.emplace_back(snapshot);
        //snapshot += region.size + sizeof(mem64_t::bytes) - 1;
        snapshot += region.size;
    }
}
