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
#include <sys/stat.h>
#include <iomanip>
#include <algorithm>
#include <dirent.h>  // deprecated
#include <filesystem>
#include <reverseengine/value.hh>
#include <reverseengine/external.hh>
#include <reverseengine/common.hh>

namespace RE {
    namespace sfs = std::filesystem;

    class Handle {
    public:
        /// Variables
        pid_t pid;
        std::string title;
        std::vector<RE::Cregion> regions_ignored;
        std::vector<RE::Cregion> regions;

        void attach(pid_t pid);

        void attach(const std::string& title);

        Handle() {
            this->pid = 0;
        }

        explicit Handle(pid_t pid) {
            attach(pid);
        }

        explicit Handle(const std::string& title) {
            attach(title);
        }


        //////////////////////////////////////////////////////////////////////
        /// Functions
        //////////////////////////////////////////////////////////////////////

        [[gnu::always_inline]]
        sfs::path get_path() {
            return sfs::canonical(sfs::current_path().root_path()/"proc"/std::to_string(pid)/"exe");
        }

        [[gnu::always_inline]]
        sfs::path get_working_directory() {
            return sfs::canonical(sfs::current_path().root_path()/"proc"/std::to_string(pid)/"cwd");
        }

        /// Checking
        [[gnu::always_inline]]
        bool is_valid() {
            return pid != 0;
        }

        // todo[med] to c++20nize
        [[gnu::always_inline]]
        bool is_running() {
            using namespace std;
            static struct stat sts{};
            errno = 0;
            return !(stat(("/proc/" + to_string(pid)).c_str(), &sts) == -1 && errno == ENOENT);
        }

        [[gnu::always_inline]]
        bool is_good() {
            return is_valid() && is_running();
        }

        /// Value returned by various member functions when they fail.
        static constexpr size_t npos = static_cast<size_t>(-1);

        /// Read_from/write_to this handle
        [[gnu::always_inline]]
        size_t
        read(uintptr_t address, void *out, size_t size) {
            static struct iovec local[1];
            static struct iovec remote[1];
            local[0].iov_base = out;
            local[0].iov_len = size;
            remote[0].iov_base = reinterpret_cast<void *>(address);
            remote[0].iov_len = size;
            return static_cast<size_t>(process_vm_readv(pid, local, 1, remote, 1, 0));
        }

        [[gnu::always_inline]]
        size_t
        write(uintptr_t address, void *in, size_t size) {
            static struct iovec local[1];
            static struct iovec remote[1];
            local[0].iov_base = in;
            local[0].iov_len = size;
            remote[0].iov_base = reinterpret_cast<void *>(address);
            remote[0].iov_len = size;
            return static_cast<size_t>(process_vm_writev(pid, local, 1, remote, 1, 0));
        }

/* ptrace peek buffer, used by peekdata() as a mirror of the process memory.
 * Max size is the maximum allowed rounded VLT scan length, aka UINT16_MAX,
 * plus a `PEEKDATA_CHUNK`, to store a full extra chunk for maneuverability */
        // FIXME[critical]: not returning negative values (possibly done)

        static constexpr size_t PEEKDATA_CHUNK = 2 * KiB;
        static constexpr size_t MAX_PEEKBUF_SIZE = 64 * KiB + PEEKDATA_CHUNK;

        uintptr_t base = 0; // base address of cached region
        std::vector<uint8_t> cache;
        size_t size; // amount of valid memory stored (in bytes)

        size_t
        inline
        read_cached(uintptr_t address, void *out, size_t size) {
            unsigned int i;
            uintptr_t missing_bytes;

            assert(cache.size() <= MAX_PEEKBUF_SIZE);

            /* check if we have a full cache hit */
            if (this->base &&
                address >= this->base &&
                (unsigned long) (address + size - this->base) <= cache.size())
            {
                out = (mem64_t *) &this->cache[address - this->base];
                return this->base - address + cache.size();
            } else if (this->base &&
                       address >= this->base &&
                       (unsigned long) (address - this->base) < cache.size())
            {
                assert(cache.size() != 0);

                /* partial hit, we have some of the data but not all, so remove old entries - shift the frame by as far as is necessary */
                missing_bytes = (address + size) - (this->base + cache.size());
                /* round up to the nearest PEEKDATA_CHUNK multiple, that is what could
                 * potentially be read and we have to fit it all */
                missing_bytes = PEEKDATA_CHUNK * (1 + (missing_bytes - 1) / PEEKDATA_CHUNK);

                /* head shift if necessary */
                if (cache.size() + missing_bytes > MAX_PEEKBUF_SIZE) {
                    uintptr_t shift_size = address - this->base;
                    shift_size = PEEKDATA_CHUNK * (shift_size / PEEKDATA_CHUNK);

                    memmove(&this->cache[0], &this->cache[shift_size], cache.size() - shift_size);

                    cache.resize(cache.size() - shift_size);
                    this->base += shift_size;
                }
            } else {
                /* cache miss, invalidate the cache */
                missing_bytes = size;
                cache.resize(0);
                this->base = address;
            }

            /* we need to retrieve memory to complete the request */
            for (i = 0; i < missing_bytes; i += PEEKDATA_CHUNK) {
                const uintptr_t target_address = this->base + cache.size();
                size_t len = this->read(target_address, &this->cache[cache.size()], PEEKDATA_CHUNK);

                /* check if the read succeeded */
                if (UNLIKELY(len < PEEKDATA_CHUNK)) {
                    if (len == 0) {
                        /* hard failure to retrieve memory */
                        return 0;
                    }
                    /* go ahead with the partial read and stop the gathering process */
                    cache.resize(cache.size() + len);
                    break;
                }
                /* otherwise, the read worked */
                cache.resize(cache.size() + PEEKDATA_CHUNK);
            }

            /* return result to caller */
            out = (mem64_t *) &this->cache[address - this->base];
            return this->base - address + cache.size();
        }


        /// Modules

        void update_regions();

        Cregion *
        get_region_by_name(const std::string& region_name) {
            for (Cregion& region : regions)
                if (region.flags & executable && region.filename == region_name)
//                  ^~~~~~~~~~~~~~~~~~~~~~~~~ wtf? fixme[medium]: add documentation or die();
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
        [[deprecated("May be moved to scanner.hh")]]
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

                if (this->read(readaddr, buffer, readsize)) {
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
            uint64_t code = 0;
            if (read(address + 1, &code, sizeof(uint32_t)) == sizeof(uint32_t))
                return code + address + 5;
            return 0;
        }

        uintptr_t
        get_absolute_address(uintptr_t address, uintptr_t offset, uintptr_t size) {
            uint64_t code = 0;
            if (read(address + offset, &code, sizeof(uint32_t)) == sizeof(uint32_t)) {
                return address + code + size;
            }
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
