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

#pragma once

#include <sys/uio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <boost/iostreams/device/mapped_file.hpp>
#include <reverseengine/value.hh>
#include <reverseengine/external.hh>
#include <reverseengine/common.hh>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <strstream> // FIXME[med]: DEPRECATED!!!


NAMESPACE_BEGIN(RE)

namespace sfs = std::filesystem;
namespace bio = boost::iostreams;


enum class region_mode_t : uint8_t
{
    none = 0,
    executable = 1u<<0u,
    writable   = 1u<<1u,
    readable   = 1u<<2u,
    shared     = 1u<<3u,
    max = executable|writable|readable|shared
};
BITMASK_DEFINE_MAX_ELEMENT(region_mode_t, max)




class Cregion {
public:
    uintptr_t address;
    uintptr_t size{};

    bitmask::bitmask<region_mode_t> flags;  // -> RE::Eregion_mode

    /// File data
    uintptr_t offset{};
    char deviceMajor{}; //fixme char?
    char deviceMinor{}; //fixme char?
    unsigned long inodeFileNumber{}; //fixme unsigned long?
    std::string pathname; //fixme pathname?
    std::string filename;

    Cregion()
    : address(0), size(0), flags(region_mode_t::none), offset(0), deviceMajor(0), deviceMinor(0), inodeFileNumber(0) {}

    bool is_good() {
        return address != 0;
    }

    std::string str() const {
        std::ostringstream ss;
        ss << "{"
           << "begin: "<<HEX(address)
           << ", end: "<<HEX(address+size)
           << ", size: "<<HEX(size)
           << ", flags: "
           << (flags & region_mode_t::shared?"s":"-")
           << (flags & region_mode_t::readable?"r":"-")
           << (flags & region_mode_t::writable?"w":"-")
           << (flags & region_mode_t::executable?"x":"-")
           << ", filename: "<<filename
           << "}";
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& outputStream, const Cregion& region) {
        return outputStream<<region.str();
    }
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & address;
        ar & size;
        ar & *reinterpret_cast<decltype(flags)::underlying_type*>(&flags);
        ar & offset;
        ar & deviceMajor;
        ar & deviceMinor;
        ar & inodeFileNumber;
        ar & pathname;
        ar & filename;
    }
};

/********************
 *  Process handler
 ********************/

class handler_i {
public:
    handler_i() = default;
    ~handler_i() = default;

    virtual size_t read(uintptr_t address, void *out, size_t size) const = 0;
    virtual size_t write(uintptr_t address, void *in, size_t size) const = 0;
    virtual bool is_valid() const = 0;
    virtual void update_regions() = 0;
    virtual const bool operator!() = 0;

    Cregion *get_region_by_name(const std::string& region_name) {
        for (Cregion& region : regions)
            if (region.flags & region_mode_t::executable && region.filename == region_name)
//                             ^~~~~~~~~~~~~~~~~~~~~~~~~ wtf? fixme[medium]: add documentation or die();
                return &region;
        return nullptr;
    }

    /*FIXME[CRITICAL]: REMAKE THIS SHIT, DO TESTS!!!!!!!!!!!!!! */
    size_t get_region_of_address(uintptr_t address) const {
        using namespace std;
        //static size_t last_return = npos;
        //if (last_return != npos
        //&& regions[last_return].address <= address
        //&& address < regions[last_return].address + regions[last_return].size) {
        //    //clog << "returning last region " << endl;
        //    return last_return;
        //}
        //cout<<"address: 0x"<<RE::HEX(address)<<", first region: 0x"<<RE::HEX(regions[0].address)<<" : 0x"<<RE::HEX(regions[0].address+regions[0].size)<<endl;
        size_t first, last, mid;
        first = 0;
        last = regions.size();
        //cout<<"last: "<<last<<endl;
        while (first <= last) {
            mid = (first + last) / 2;
            //cout<<"   mid: "<<mid<<endl;
            if (address < regions[mid].address) {
                last = mid - 1;
                //cout<<"      address at the left side of 0x"<<RE::HEX(regions[mid].address)<<endl;
            } else if (address > regions[mid].address + regions[mid].size) {
                first = mid + 1;
                //cout<<"      address at the right side of 0x"<<RE::HEX(regions[mid].address + regions[mid].size)<<endl;
            } else {
                //last_return = mid;
                //return last_return;
                //std::cout<<"      returning mid: "<<mid<<std::endl;
                return mid;
            }
        }
        //std::cout<<"      returning npos: "<<mid<<std::endl;
        return npos;
    }

    uintptr_t get_call_address(uintptr_t address) const {
        uint64_t code = 0;
        if (read(address + 1, &code, sizeof(uint32_t)) == sizeof(uint32_t))
            return code + address + 5;
        return 0;
    }

    uintptr_t get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) const {
        uint64_t code = 0;
        if (read(address + offset, &code, sizeof(uint32_t)) == sizeof(uint32_t)) {
            return address + code + size;
        }
        return 0;
    }

public:
    /// Value returned by various member functions when they fail.
    static constexpr size_t npos = static_cast<size_t>(-1);

    std::vector<Cregion> regions;
    std::vector<Cregion> regions_ignored;
};


class handler_pid : public handler_i {
public:
    using handler_i::handler_i;

    handler_pid() : m_pid(0) {}

    /** Make process handler from PID */
    explicit handler_pid(pid_t pid) noexcept {
        m_pid = pid;
        try {
            m_title = get_executable().filename();
        } catch (const sfs::filesystem_error& e) {
            /* no such PID running */
            ;
        }
        if (m_title.empty()) {
            m_pid = 0;
            m_title = "";
            return;
        }
    }

    /** Make process handler from process executable filename */
    explicit handler_pid(const std::string& title) {
        m_pid = 0;
        if (title.empty())
            return;

        try {
            for(auto& p: sfs::directory_iterator("/proc")) {
                if (!sfs::is_directory(p))
                    continue;
                if (!sfs::exists(p / "exe"))
                    /* if process is not an executable (e.g. TODO[critical]: add example ) */
                    continue;
                if (!sfs::exists(p / "maps"))
                    continue;
                if (p.path().filename().string().find_first_not_of("0123456789") != std::string::npos)
                    /* if filename is not numeric */
                    continue;

                // TODO[high]: rewrite, because is_valid and is_running is already checked
                std::istringstream str_pid(p.path().filename().string());
                str_pid >> m_pid;
                if (!*this)
                    /* if process not exist */
                    continue;

                // todo[medium]: okay, we got alot of chromium, but which chromium is on top of others? Hint: cat /proc/1234/cmdline
                if (sfs::canonical(p / "exe").filename() == title) {
                    m_title = title;
                    return;
                }
                m_pid = 0;
            }
        } catch (const sfs::filesystem_error& e) {
            /* not linux or linux have no "/proc" folder */
        }
    }

    /** Process executable real path */
    // todo[medium] it SHOULD throw something if [[asdad]] proccess with pid '4' attached
    [[gnu::always_inline]]
    sfs::path get_executable() {
        return sfs::canonical(sfs::path("/proc") / std::to_string(m_pid) / "exe");
    }

    /** Process working directory real path */
    [[gnu::always_inline]]
    sfs::path get_working_directory() {
        return sfs::canonical(sfs::path("/proc") / std::to_string(m_pid) / "cwd");
    }

    /** Checking */
    [[gnu::always_inline]]
    bool is_valid() const override {
        return m_pid != 0;
    }

    /** Process working directory real path */
    [[gnu::always_inline]]
    bool is_running() const {
        return sfs::exists(sfs::path("/proc") / std::to_string(m_pid));
    }

    /** Read value */
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) const override {
        struct iovec local[1];
        struct iovec remote[1];
        local[0].iov_base = out;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        return static_cast<size_t>(process_vm_readv(m_pid, local, 1, remote, 1, 0));
    }

    /** Write value */
    [[gnu::always_inline]]
    size_t write(uintptr_t address, void *in, size_t size) const override {
        struct iovec local[1];
        struct iovec remote[1];
        local[0].iov_base = in;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void *>(address);
        remote[0].iov_len = size;
        return static_cast<size_t>(process_vm_writev(m_pid, local, 1, remote, 1, 0));
    }

    ///** Read value, return true if success */
    //template <typename T, size_t _SIZE = sizeof(T)>
    //bool read(uintptr_t address, T *out) {
    //    return read(address, out, _SIZE) == _SIZE;
    //}
    //
    ///** Read value, return true if success */
    //template <typename T, size_t _SIZE = sizeof(T)>
    //bool write(uintptr_t address, T *in) {
    //    return write(address, in, _SIZE) == _SIZE;
    //}

    void update_regions() override {
        regions.clear();
        regions_ignored.clear();
        std::ifstream maps(sfs::path("/proc") / std::to_string(get_pid()) / "maps");
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

    constexpr pid_t get_pid() { return this->m_pid; }

    std::string get_title() {
        m_title = sfs::canonical(sfs::path("/proc") / std::to_string(m_pid) / "exe").filename();
        return m_title;
    }

    const bool operator!() override { return !is_valid() || !is_running(); }

private:
    pid_t m_pid;
    std::string m_title;
};

//class handler_pid_cached : public handler_pid {
//public:
//    using handler_pid::handler_pid;
//
//    handler_pid_cached() = delete;
//
//    explicit handler_pid_cached(pid_t pid)
//    : handler_pid(pid) {
//        std::cout<<"handler_pid_cached(pid_t)"<<std::endl;
//        m_base = 0;
//        m_cache_size = 0;
//        m_cache = new uint8_t[MAX_PEEKBUF_SIZE];
//    }
//
//    explicit handler_pid_cached(const std::string& target)
//    : handler_pid(target) {
//        std::cout<<"handler_pid_cached(const std::string&)"<<std::endl;
//        m_base = 0;
//        m_cache_size = 0;
//        m_cache = new uint8_t[MAX_PEEKBUF_SIZE];
//    }
//
//    ~handler_pid_cached() {
//        std::cout<<"~handler_pid_cached()"<<std::endl;
//        delete[] m_cache;
//    }
//
//    [[gnu::always_inline]]
//    void reset() {
//        m_base = 0;
//        m_cache_size = 0;
//    }
//
//    [[gnu::always_inline]]
//    size_t read(uintptr_t address, void *out, size_t size) {
//        if UNLIKELY(size > MAX_PEEKBUF_SIZE) {
//            return handler_pid::read(address, out, size);
//        }
//
//        if (m_base
//            && address >= m_base
//            && address - m_base + size <= m_cache_size) {
//            /* full cache hit */
//            memcpy(out, &m_cache[address - m_base], size);
//            return size;
//        }
//
//        /* we need to retrieve memory to complete the request */
//        size_t len = handler_pid::read(address, &m_cache[0], MAX_PEEKBUF_SIZE);
//        if (len == handler_pid::npos) {
//            /* hard failure to retrieve memory */
//            reset();
//            return handler_pid::npos;
//        }
//
//        m_base = address;
//        m_cache_size = len;
//
//        /* return result to caller */
//        memcpy(out, &m_cache[0], size);
//        return MIN(size, m_cache_size);
//    }
//
//public:
//    static constexpr size_t MAX_PEEKBUF_SIZE = 4 * (1 << 10);
//
//private:
//    uintptr_t m_base; // base address of cached region
//    size_t m_cache_size;
//    uint8_t *m_cache;
//};

/*!
 * @brief  Used to store size of objects to be serialized
 */
class counter_streambuf : public std::streambuf
{
public:
    using std::streambuf::streambuf;

    size_t size() const { return m_size; }

protected:
    std::streamsize xsputn(const char_type* __s, std::streamsize __n) override
    { this->m_size += __n; return __n; }

private:
    size_t m_size = 0;
};

class handler_mmap : public handler_i {
public:
    using handler_i::handler_i;

    void save(const handler_pid& handler, const std::string& path) {
        regions = handler.regions;
        size_t total_scan_bytes = 0;
        for (const RE::Cregion& r : handler.regions) {
            total_scan_bytes += r.size;
        }

        RE::counter_streambuf csb;
        boost::archive::binary_oarchive boa_csb(csb, boost::archive::no_header);
        boa_csb << *this;

        snapshot_mf_.path = path;
        snapshot_mf_.flags = bio::mapped_file::mapmode::readwrite;
        snapshot_mf_.new_file_size = total_scan_bytes + csb.size();
        snapshot_mf.open(snapshot_mf_);
        if (!snapshot_mf.is_open())
            throw std::invalid_argument("can not open '" + path + "'");

        char *snapshot = snapshot_mf.data();

        std::ostrstream oss(snapshot, csb.size());
        boost::archive::binary_oarchive boa_oss(oss, boost::archive::no_header);
        boa_oss << *this;
        oss.flush();
        snapshot += csb.size();

        regions_on_map.clear();
        for(RE::Cregion region : handler.regions) {
            regions_on_map.emplace_back(snapshot);
            ssize_t copied = handler.read(region.address, snapshot, region.size);
            snapshot += region.size;
        }
        //todo[low]: maybe we should made `mmap_streambuf` as `counter_streambuf` made?
        //snapshot_mf.resize(snapshot - snapshot_mf.data());
    }



    /// Open (open snapshot from file)
    explicit handler_mmap(const std::string& path) {
        std::ifstream ifs(path);
        boost::archive::binary_iarchive ia(ifs);
        // read class state from archive

        std::string magic_bytes;
        uint8_t a;
        ia >> regions;
    }

    /// Save (make snapshot)
    explicit handler_mmap(const handler_pid& handler, const std::string& path) {
        save(handler, path);
    }


    /** Read value */
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) const override {
        size_t r = get_region_of_address(address);
        if UNLIKELY(r == npos)
            throw std::invalid_argument("read: cannot find region of address "+RE::HEX(address));
        memcpy(out, reinterpret_cast<char *>(regions_on_map[r] + address - regions[r].address), size);
        return static_cast<size_t>(size);
    }

    /** Write value */
    [[gnu::always_inline]]
    size_t write(uintptr_t address, void *in, size_t size) const override {
        return static_cast<size_t>(size);
    }

    bool is_valid() const override {
        return false;
    }

    void update_regions() override {

    }

    const bool operator!() override {
        return 0;
    }

private:
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & regions;
    }

private:
    bio::mapped_file snapshot_mf;
    bio::mapped_file_params snapshot_mf_;
    std::vector<char *> regions_on_map;
};


/*bool //todo короче можно вместо стрима закинуть, например, вектор со стрингами
handler::findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, size_t point_size, std::ostream *ss)
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
handler::findPattern(vector<uintptr_t> *out, Cregion *region, const char *pattern, const char *mask)
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
handler::scan_exact(vector<Entry> *out,
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


NAMESPACE_END(RE)

//[[deprecated("May be moved to scanner.hh")]]
//bool find_pattern(uintptr_t *out, Cregion *region, const char *pattern, const char *mask) {
//    char buffer[0x1000];
//
//    uintptr_t len = strlen(mask);
//    uintptr_t chunksize = sizeof(buffer);
//    uintptr_t totalsize = region->size;
//    uintptr_t chunknum = 0;
//
//    while (totalsize) {
//        uintptr_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        uintptr_t readaddr = region->address + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//
//        if (this->read(readaddr, buffer, readsize)) {
//            for (uintptr_t b = 0; b < readsize; b++) {
//                uintptr_t matches = 0;
//
//                // если данные совпадают или пропустить
//                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
//                    matches++;
//
//                    if (matches == len) {
//                        *out = readaddr + b;
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