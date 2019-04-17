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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <reverseengine/r2_wrapper.hh>
#include <reverseengine/value.hh>
#include <reverseengine/external.hh>
#include <reverseengine/common.hh>



NAMESPACE_BEGIN(RE)

namespace sfs = std::filesystem;
namespace bio = boost::iostreams;


enum class region_mode_t : uint8_t
{
    none = 0u,
    executable = 1u<<0u,
    writable   = 1u<<1u,
    readable   = 1u<<2u,
    shared     = 1u<<3u,
    max = 0u|executable|writable|readable|shared
};
BITMASK_DEFINE_VALUE_MASK(region_mode_t, 0xff)


class Region {
public:
    uintptr_t address;
    uintptr_t size;
    bitmask::bitmask<region_mode_t> flags;

    /// File data
    uintptr_t offset;
    union {
        struct {
            uint8_t st_device_minor;
            uint8_t st_device_major;
        };
        __dev_t st_device;
    };
    uint64_t inode;
    sfs::path file;
    R2::Bin r2;

    Region()
    : address(0), size(0), flags(region_mode_t::none), offset(0), st_device(0), inode(0) { }

    ~Region() = default;

    bool is_good() {
        return address != 0;
    }

    std::string str() const {
        std::ostringstream ss;
        ss << HEX(address)<<"-"<<HEX(address+size)<<" ("<<HEX(size)<<") "
           << (flags & region_mode_t::shared?"s":"-") << (flags & region_mode_t::readable?"r":"-") << (flags & region_mode_t::writable?"w":"-") << (flags & region_mode_t::executable?"x":"-") << " "
           << HEX(offset) << " "
           << HEX(st_device_major)<<":"<< HEX(st_device_minor) <<" "
           << inode<<" "
           << file;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& outputStream, const Region& region) {
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
        // ar & file;
    }
};

/********************
 *  Process handler
 ********************/

class IProcess {
public:
    IProcess() = default;
    ~IProcess() = default;

    virtual size_t read(uintptr_t address, void *out, size_t size) const = 0;
    virtual size_t write(uintptr_t address, void *in, size_t size) const = 0;
    virtual bool is_valid() const = 0;
    virtual void update_regions() = 0;
    virtual const bool operator!() = 0;

    Region *get_region_by_name(const std::string& region_name);
    Region *get_region_of_address(uintptr_t address) const;
    uintptr_t get_call_address(uintptr_t address) const;
    uintptr_t get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) const;

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

public:
    /// Value returned by various member functions when they fail.
    static constexpr size_t npos = static_cast<size_t>(-1);

    std::vector<Region> regions;
};


/** Process - process is active */
class Process : public IProcess {
public:
    using IProcess::IProcess;
    using IProcess::read;

    Process() : m_pid(0) {}

    /** Make process handler from PID */
    explicit Process(pid_t pid) noexcept;

    /** Make process handler from process executable filename */
    explicit Process(const std::string& title);

    /*!
     * Returns a symlink to the original executable file, if it still exists
     * (a process may continue running after its original executable has been deleted or replaced).
     * If the pathname has been unlinked, the symbolic link will contain the string ' (deleted)' appended to the original pathname.
     */
    [[gnu::always_inline]]
    sfs::path get_exe() { return sfs::read_symlink(sfs::path("/proc") / std::to_string(m_pid) / "exe"); }

    /*!
     * Returns path to the original executable file, if it still exists, or throws std::runtime_error("File not found").
     */
    sfs::path get_executable();

    /** Process working directory real path */
    [[gnu::always_inline]]
    sfs::path get_working_directory() { return sfs::read_symlink(sfs::path("/proc") / std::to_string(m_pid) / "cwd"); }

    /** Checking */
    [[gnu::always_inline]]
    bool is_valid() const override { return m_pid != 0; }

    /** Process working directory real path */
    [[gnu::always_inline]]
    bool is_running() const { return sfs::exists(sfs::path("/proc") / std::to_string(m_pid)); }

    size_t read(uintptr_t address, void *out, size_t size) const override;
    size_t write(uintptr_t address, void *in, size_t size) const override;
    void update_regions() override;
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

/** IProcessM - memory mapped reader */
class IProcessM : public IProcess {
public:
    using IProcess::IProcess;

    size_t read(uintptr_t address, void *out, size_t size) const override;
    size_t write(uintptr_t address, void *in, size_t size) const override;
    bool is_valid() const override;
    void update_regions() override;
    const bool operator!() override;

public:
    std::vector<char *> regions_on_map;
};

class ProcessF;

/**
 * IProcessM - memory mapped reader
 * ProcessH - process loaded to HEAP
 */
class ProcessH : public IProcessM {
public:
    using IProcessM::IProcessM;

    explicit ProcessH(const Process& rhs) {
        regions = rhs.regions;
        for (const RE::Region& region : rhs.regions) {
            char* map = new char[region.size];
            regions_on_map.emplace_back(map);
            ssize_t copied = rhs.read(region.address, map, region.size);
        }
    }

    explicit ProcessH(const ProcessF& rhs);

    ~ProcessH() {
        for (char* r : regions_on_map) {
            if (r) {
                delete[] r;
                r = nullptr;
            }
        }
        regions_on_map.clear();
    }

};

/**
 * IProcessM - memory mapped reader
 * ProcessF - process file memory mapped
 */
class ProcessF : public IProcessM {
public:
    using IProcessM::IProcessM;

    /*!
     * Now, we will able to send data to another PC and search pointers together.
     */
    void
    save(const Process& handler, const std::string& path) {
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
        assert(stream.tellg() != -1);
        snapshot += stream.tellg();

        regions_on_map.clear();
        for (const RE::Region& region : regions) {
            regions_on_map.emplace_back(snapshot);
            //snapshot += region.size + sizeof(mem64_t::bytes) - 1;
            snapshot += region.size;
        }
    }

    /// Open (open snapshot from file)
    explicit ProcessF(const std::string& path) {
        this->load(path);
    }

    ~ProcessF() = default;

    /// Save (make snapshot)
    explicit ProcessF(const Process& handler, const std::string& path) {
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

    template<class PH = __PHANDLER, typename std::enable_if<std::is_base_of<PH, ProcessF>::value>::type* = nullptr>
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

    template<class PH = __PHANDLER, typename std::enable_if<!std::is_base_of<PH, ProcessF>::value>::type* = nullptr>
    [[gnu::always_inline]]
    size_t read(uintptr_t address, void *out, size_t size) {
        return m_parent.read(address, out, size);
    }

public:
    static constexpr size_t MAX_PEEKBUF_SIZE = 4u * (1u << 10u);

private:
    __PHANDLER& m_parent;
    uintptr_t m_base; // base address of cached region
    size_t m_cache_size;
    uint8_t *m_cache;
};

NAMESPACE_END(RE)
