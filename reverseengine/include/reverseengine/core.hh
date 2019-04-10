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
#include <regex>
#include <sys/stat.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <reverseengine/value.hh>
#include <reverseengine/external.hh>
#include <reverseengine/common.hh>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <r_core.h>
#include <r_types.h>
#include <r_util.h>
#include <r_bin.h>
#include <r_io.h>



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






class region {
public:
    uintptr_t address;
    uintptr_t size;
    bitmask::bitmask<region_mode_t> flags;

    /// File data
    RCore *b;
    uintptr_t offset;
    union {
        struct {
            uint8_t st_device_minor;
            uint8_t st_device_major;
        };
        __dev_t st_device;
    };
    uint64_t inode;
    std::string pathname;
    std::string filename;

    region()
    : address(0), size(0), flags(region_mode_t::none), offset(0), st_device(0), inode(0) {

    }

    bool is_good() {
        return address != 0;
    }

    std::string str() const {
        std::ostringstream ss;
        ss << HEX(address)<<"-"<<HEX(address+size)<<" ("<<HEX(size)<<") "
           << (flags & region_mode_t::shared?"s":"-") << (flags & region_mode_t::readable?"r":"-") << (flags & region_mode_t::writable?"w":"-") << (flags & region_mode_t::executable?"x":"-") << " "
           << HEX(st_device_major)<<":"<< HEX(st_device_minor) <<" "
           << inode<<" "
           << pathname;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& outputStream, const region& region) {
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
        ar & st_device_minor;
        ar & st_device_major;
        ar & inode;
        ar & pathname;
        ar & filename;
    }
};

/********************
 *  Process handler
 ********************/

class phandler_i {
public:
    phandler_i() = default;
    ~phandler_i() = default;

    virtual size_t read(uintptr_t address, void *out, size_t size) const = 0;
    virtual size_t write(uintptr_t address, void *in, size_t size) const = 0;
    virtual bool is_valid() const = 0;
    virtual void update_regions() = 0;
    virtual const bool operator!() = 0;

    ///** Read value, return true if success */
    template <typename T>
    bool read(uintptr_t address, T *out) {
        return read(address, out, sizeof(T)) == sizeof(T);
    }

    ///** Read value, return value if success, throws on error */
    template <typename T>
    T read(uintptr_t address) {
        T a;
        if (read(address, &a, sizeof(T)) == sizeof(T))
            return a;
        else
            throw std::runtime_error("Cannot read "+HEX(address));
    }

    ///** Read value, return true if success */
    template <typename T>
    bool write(uintptr_t address, T *in) {
        return write(address, in, sizeof(T)) == sizeof(T);
    }

    region *
    get_region_by_name(const std::string& region_name);

    region *
    get_region_of_address(uintptr_t address) const;

    uintptr_t
    get_call_address(uintptr_t address) const;

    uintptr_t
    get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) const;

public:
    /// Value returned by various member functions when they fail.
    static constexpr size_t npos = static_cast<size_t>(-1);

    std::vector<region> regions;
};


class phandler_pid : public phandler_i {
public:
    using phandler_i::phandler_i;
    using phandler_i::read;

    phandler_pid() : m_pid(0) {}

    /** Make process handler from PID */
    explicit phandler_pid(pid_t pid) noexcept;

    /** Make process handler from process executable filename */
    explicit phandler_pid(const std::string& title);

    /*!
     * Returns a symlink to the original executable file, if it still exists
     * (a process may continue running after its original executable has been deleted or replaced).
     * If the pathname has been unlinked, the symbolic link will contain the string ' (deleted)' appended to the original pathname.
     */
    [[gnu::always_inline]]
    sfs::path
    get_exe() {
        return sfs::read_symlink(sfs::path("/proc") / std::to_string(m_pid) / "exe");
    }

    /*!
     * Returns path to the original executable file, if it still exists, or throws std::runtime_error("File not found").
     */
    [[gnu::always_inline]]
    sfs::path
    get_executable() {
        /* Ambiguous */
        std::string exe = get_exe();
        for (const region& region : regions) {
            if (region.pathname == exe) {
                struct stat sb{};
                errno = 0;
                int s = stat(exe.c_str(), &sb);
                if (s == 0 && sb.st_dev == region.st_device && sb.st_ino == region.inode && region.offset == 0)
                    return sfs::path(exe);
            }
        }
        throw std::runtime_error("File not found");
    }

    /** Process working directory real path */
    [[gnu::always_inline]]
    sfs::path get_working_directory() {
        return sfs::read_symlink(sfs::path("/proc") / std::to_string(m_pid) / "cwd");
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

    void update_regions() override {
        regions.clear();
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

                region region;

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
                    region.st_device_major = static_cast<uint8_t>(stoi(device.substr(0, deviceSplit), nullptr, 16));
                    region.st_device_minor = static_cast<uint8_t>(stoi(device.substr(deviceSplit + 1, device.size()), nullptr, 16));
                }

                ss << std::hex << offset;
                ss >> region.offset;
                ss.clear();
                ss << std::dec << inode;
                ss >> region.inode;

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
                regions.push_back(region);
            }
        }
        regions.shrink_to_fit();
    }

    constexpr pid_t get_pid() { return this->m_pid; }

    /** /proc/pid/cmdline
     * @return std::string, because cmdline is not null-terminated, e.g.:
     * "/usr/bin/qemu-system-x86_64\x00-name\x00guest=generic,debug-threads=on\x00"
     */
    [[gnu::always_inline]]
    std::string get_cmdline() {
        std::ifstream t(sfs::path("/proc") / std::to_string(m_pid) / "cmdline");
        std::ostringstream ss;
        ss<<t.rdbuf();
        m_cmdline = ss.str();
        return m_cmdline;
    }

    const bool operator!() override { return !is_valid() || !is_running(); }

private:
    pid_t m_pid;
    std::string m_cmdline;
};

class phandler_map_i : public phandler_i {
public:
    using phandler_i::phandler_i;

    /** Read value */
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) const override {
        using namespace std;
        region *r = get_region_of_address(address);
        if UNLIKELY(r == nullptr)
            return npos;
        if UNLIKELY(address + size > r->address + r->size) {
            size = r->size - (address - r->address);
        }
        memcpy(out, reinterpret_cast<char *>(regions_on_map[r - &regions[0]] + (address - r->address)), size);
        return size;
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

public:
    std::vector<char *> regions_on_map;
};

class phandler_file;

class phandler_memory : public phandler_map_i {
public:
    using phandler_map_i::phandler_map_i;

    explicit phandler_memory(const phandler_pid& rhs) {
        regions = rhs.regions;
        for (const RE::region& region : rhs.regions) {
            char* map = new char[region.size];
            regions_on_map.emplace_back(map);
            ssize_t copied = rhs.read(region.address, map, region.size);
        }
    }

    explicit phandler_memory(const phandler_file& rhs);

    ~phandler_memory() {
        for (char* r : regions_on_map) {
            if (r) {
                delete[] r;
                r = nullptr;
            }
        }
        regions_on_map.clear();
    }

};


class phandler_file : public phandler_map_i {
public:
    using phandler_map_i::phandler_map_i;

    /*!
     * Now, we will able to send data to another PC and search pointers together.
     */
    void
    save(const phandler_pid& handler, const std::string& path) {
        regions = handler.regions;

        std::ofstream stream(path, std::ios_base::out | std::ios_base::binary);
        boost::archive::binary_oarchive archive(stream, boost::archive::no_header);
        archive << *this;
        stream.flush();

        params.path = path;
        params.flags = bio::mapped_file::mapmode::readwrite;
        params.new_file_size = 0; //todo[critical]: to remove??
        mf.open(params);
        if (!mf.is_open())
            throw std::invalid_argument("can not open '" + path + "'");

        size_t bytes_to_save = 0;
        for (const RE::region& region : handler.regions)
            bytes_to_save += sizeof(region.size) + region.size;
        mf.resize(mf.size() + bytes_to_save);

        char* snapshot = mf.data();
        assert(stream.is_open());
        assert(stream.tellp() != -1);
        snapshot += stream.tellp();

        regions_on_map.clear();
        for(const RE::region& region : handler.regions) {
            regions_on_map.emplace_back(snapshot);
            ssize_t copied = handler.read(region.address, snapshot, region.size);
            //snapshot += region.size + sizeof(mem64_t::bytes) - 1;
            snapshot += region.size;
        }
    }

    void
    load(const std::string& path) {
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
        assert(stream.tellp() != -1);
        snapshot += stream.tellg();

        regions_on_map.clear();
        for (const RE::region& region : regions) {
            regions_on_map.emplace_back(snapshot);
            //snapshot += region.size + sizeof(mem64_t::bytes) - 1;
            snapshot += region.size;
        }
    }

    /// Open (open snapshot from file)
    explicit phandler_file(const std::string& path) {
        this->load(path);
    }

    ~phandler_file() {
    }

    /// Save (make snapshot)
    explicit phandler_file(const phandler_pid& handler, const std::string& path) {
        this->save(handler, path);
    }

    bool is_valid() const override {
        return false;
    }

    void update_regions() override {

    }

    const bool operator!() override {
        return 0;
    }

public:
    const std::string magic = "RE::phandler_file";

private:
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & regions;
    }

private:
    bio::mapped_file mf;
    bio::mapped_file_params params;
};


/** The cached reader made for reading small values many times to reduce system calls */
template<class __PHANDLER>
class cached_reader {
public:
    explicit cached_reader(__PHANDLER& parent)
            : m_parent(parent) {
        m_base = 0;
        m_cache_size = 0;
        m_cache = new uint8_t[MAX_PEEKBUF_SIZE];
    }

    ~cached_reader() {
        delete[] m_cache;
    }

    [[gnu::always_inline]]
    void reset() {
        m_base = 0;
        m_cache_size = 0;
    }

    template<class PH = __PHANDLER, typename std::enable_if<std::is_base_of<PH, phandler_file>::value>::type* = nullptr>
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) {
        if UNLIKELY(size > MAX_PEEKBUF_SIZE) {
            return m_parent.read(address, out, size);
        }

        if (m_base
            && address >= m_base
            && address - m_base + size <= m_cache_size) {
            /* full cache hit */
            memcpy(out, &m_cache[address - m_base], size);
            return size;
        }

        /* we need to retrieve memory to complete the request */
        size_t len = m_parent.read(address, &m_cache[0], MAX_PEEKBUF_SIZE);
        if (len == m_parent.npos) {
            /* hard failure to retrieve memory */
            reset();
            return m_parent.npos;
        }

        m_base = address;
        m_cache_size = len;

        /* return result to caller */
        memcpy(out, &m_cache[0], size);
        return MIN(size, m_cache_size);
    }

    template<class PH = __PHANDLER, typename std::enable_if<!std::is_base_of<PH, phandler_file>::value>::type* = nullptr>
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) {
        return m_parent.read(address, out, size);
    }

public:
    static constexpr size_t MAX_PEEKBUF_SIZE = 4 * (1 << 10);

private:
    __PHANDLER& m_parent;
    uintptr_t m_base; // base address of cached region
    size_t m_cache_size;
    uint8_t *m_cache;
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
handler::findPattern(vector<uintptr_t> *out, region *region, const char *pattern, const char *mask)
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
                   const region *region,
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
//bool find_pattern(uintptr_t *out, region *region, const char *pattern, const char *mask) {
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
