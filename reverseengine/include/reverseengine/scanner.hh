/*
    This file is part of Reverse Engine.

    Array of scanner results.

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>
    Copyright (C) 2015,2017 Sebastian Parschauer <s.parschauer@gmx.de>
    Copyright (C) 2017      Andrea Stacchiotti <andreastacchiotti@gmail.com>
    Copyright (C) 2010      WANG Lu <coolwanglu@gmail.com>
    Copyright (C) 2009      Eli Dupree <elidupree@harter.net>

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

#include <cinttypes>
#include <memory>
#include <cmath>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <chrono>
#include <deque>
#include <string>
//#include <boost/iostreams/device/mapped_file.hpp>
#include <cstdlib>
//
#include <reverseengine/core.hh>
#include <reverseengine/value.hh>
#include <reverseengine/scanroutines.hh>
#include <reverseengine/pointerscan.hh>
#include <omp.h>

//namespace bio = boost::iostreams;

NAMESPACE_BEGIN(RE)

/**
 *  Better to have multiple little arrays called 'swath_t', than one large array of bytes and flags.
 *  E.g. target program have array of uint32_t full of zeros and ones:
 *      00010010100000100000000000000000010010111111000
 *  and we wanted to match only '1':
 *      00010010100000100000000000000000010010111111000
 *         ^~ ^~~~    ^~                 ^~~~~~~~~~~~
 *  Using 'swath_t' we will use less memory space.
 */
class swath_t
{
public:
    uintptr_t base_address;
    std::vector<uint8_t> bytes;
    std::vector<flag> flags;

    explicit swath_t(uintptr_t base_address) {
        this->base_address = base_address;
    }

    [[gnu::always_inline]]
    void
    append(uint8_t byte, const flag& flag) {
        bytes.emplace_back(byte);
        flags.emplace_back(flag);
    }

    [[gnu::always_inline]]
    uintptr_t
    remote_get(size_t n) {
        return base_address + static_cast<uintptr_t>(n);
    }

    [[gnu::always_inline]]
    uintptr_t
    remote_back() {
        return remote_get(bytes.size() - 1);
    }

/* only at most sizeof(int64_t) bytes will be read,
   if more bytes are needed (e.g. bytearray),
   read them separately (for performance) */
    [[gnu::always_inline]]
    value_t
    to_value(size_t index)
    {
        //todo[high]: do constructor like value_t(swath_t)
        value_t val;

        val.address = remote_get(index);

        /* Truncate to the old flags, which are stored with the first matched byte */
        val.flags = flags[index];

        size_t max_bytes = bytes.size() - index;

        /* NOTE: This does the right thing for VLT because the flags are in
         * the same order as the number representation (for both endians), so
         * that the zeroing of a flag does not change useful bits of `length`. */
        if (max_bytes > 8)
            max_bytes = 8;

        memcpy(val.bytes, &bytes[index], max_bytes);

        return val;
    }
};


/**
 *  Master array of matches array.
 */
class matches_t {
public:
    //left side
    //std::ostream file;
    //right side
    std::vector<swath_t> swaths;

    matches_t() { }

    ~matches_t() { }

    [[gnu::always_inline]]
    void
    add_element(const uintptr_t& remote_address, const mem64_t *remote_memory, const flag& flags)
    {
        if UNLIKELY(remote_address == 0)
            throw std::bad_exception();
        if UNLIKELY(swaths.empty()) {
            swaths.emplace_back(remote_address);
        }

        constexpr size_t preallocated_bytes = 1;
        constexpr size_t size_of_bytes_flags = preallocated_bytes*(sizeof(decltype(swath_t::bytes)::value_type) + sizeof(decltype(swath_t::flags)::value_type));
        size_t remote_delta = remote_address - swaths.back().remote_back();
        size_t local_delta = remote_delta * size_of_bytes_flags;

        if (local_delta >= sizeof(swath_t) + size_of_bytes_flags) {
            /* It is more memory-efficient to start a new swath */
            swaths.emplace_back(remote_address);
        } else {
            /* It is more memory-efficient to write over the intervening space with null values */
            while (remote_delta-- > 1)
                swaths.back().append(0, RE::flag(RE::flag_t::flags_empty));
        }
        swaths.back().append(remote_memory->u8, flags);
        assert(swaths.back().flags.size() == swaths.back().bytes.size());
    }

    size_t mem_allo() {
        size_t size = 0;
        for(auto& swath : swaths)
            size += swath.bytes.capacity() * sizeof(decltype(swath.bytes)::value_type)
                    + swath.flags.capacity() * sizeof(decltype(swath.flags)::value_type);
        return size + swaths.capacity() * sizeof(swath_t);
    }

    size_t mem_virt() {
        size_t size = 0;
        for(auto& swath : swaths)
            size += swath.bytes.size() * sizeof(decltype(swath.bytes)::value_type)
                    + swath.flags.size() * sizeof(decltype(swath.flags)::value_type);
        return size + swaths.size() * sizeof(swath_t);
    }

    size_t mem_disk() {
        size_t size = 0;
        for(auto& swath : swaths)
            size += swath.bytes.size() * sizeof(decltype(swath.bytes)::value_type)
                    + swath.flags.size() * sizeof(decltype(swath.flags)::value_type);
        return size + swaths.size() * sizeof(swath_t);
    }

    size_t size() {
        size_t matches = 0;
        for(auto& swath : swaths)
            for(auto& f : swath.flags)
                matches++;
        return matches;
    }

    size_t count() {
        size_t valid_matches = 0;
        for(auto& swath : swaths)
            for(auto& f : swath.flags)
                if (f != flag_t::flags_empty)
                    valid_matches++;
        return valid_matches;
    }


    struct iterator {
        typedef value_t value_type;
        typedef value_t& reference_type;
        typedef value_t* pointer_type;
        typedef value_t difference_type;
        typedef std::output_iterator_tag iterator_category;

        iterator& operator++() {
            using namespace std;
            value_index++;
            for(; swath_index < parent->swaths.size(); swath_index++, value_index = 0) {
                for(; value_index < parent->swaths[swath_index].flags.size(); value_index++) {
                    if (parent->swaths[swath_index].flags[value_index] != RE::flag_t::flags_empty) {
                        //cout<<"swath_index: "<<swath_index<<" (size "<<parent->swaths.size()<<"), value_index: "<<value_index<<" (size "<<parent->swaths[swath_index].bytes.size()<<"), ++ is found"<<endl;
                        return *this;
                    }
                    //cout<<"swath_index: "<<swath_index<<" (size "<<parent->swaths.size()<<"), value_index: "<<value_index<<" (size "<<parent->swaths[swath_index].bytes.size()<<"), ++ is skipping"<<endl;
                }
            }
            swath_index = parent->swaths.size();
            value_index = 0;
            return *this;
        }

        const iterator operator++(int) const {
            iterator it(parent, swath_index, value_index);
            ++it;
            return it;
        }

        value_type operator*() const {
            //cout<<"swath_index: "<<swath_index<<" (size "<<parent->swaths.size()<<"), value_index: "<<value_index<<" (size "<<parent->swaths[swath_index].bytes.size()<<"), dereference"<<endl;
            if (swath_index >= parent->swaths.size() || value_index >= parent->swaths[swath_index].bytes.size()) {
                throw std::range_error("matches_t: dereference outside of range");
            }
            return parent->swaths[swath_index].to_value(value_index);
        }

        friend void swap(iterator& lhs, iterator& rhs) {
            std::swap(lhs.parent, rhs.parent);
            std::swap(lhs.swath_index, rhs.swath_index);
            std::swap(lhs.value_index, rhs.value_index);
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.parent==rhs.parent && lhs.swath_index==rhs.swath_index && lhs.value_index==rhs.value_index;
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs) {
            bool ret = lhs.parent!=rhs.parent || lhs.swath_index!=rhs.swath_index || lhs.value_index!=rhs.value_index;
            return ret;
        }
    private:
        friend matches_t;
        matches_t *parent;
        size_t swath_index;
        size_t value_index;

        explicit iterator(matches_t* container, size_t swath = 0, size_t value = 0) : parent(container), swath_index(swath), value_index(value) {
            for (; swath_index < parent->swaths.size(); swath_index++) {
                for (size_t d = 0; d < parent->swaths[swath_index].flags.size(); d++) {
                    if (parent->swaths[swath_index].flags[d] == RE::flag_t::flags_empty)
                        continue;
                    value_index = d;
                    //cout<<"swath_index: "<<swath_index<<" (size "<<parent->swaths.size()<<"), value_index: "<<value_index<<" (size "<<parent->swaths[swath_index].bytes.size()<<"), begin iterator"<<endl;
                    return;
                }
            }
            //cout<<"swath_index: "<<swath_index<<" (size "<<parent->swaths.size()<<"), value_index: "<<value_index<<" (size "<<parent->swaths[swath_index].bytes.size()<<"), end iterator"<<endl;
        }
    };

    iterator begin() { return iterator(this); }

    iterator end() { return iterator(this, this->swaths.size()); }

    const value_t operator[](size_t key) {
        size_t i = 0;
        for (size_t s = 0; s < swaths.size(); s++) {
            for (size_t d = 0; d < swaths[s].flags.size(); d++) {
                if (swaths[s].flags[d] == RE::flag_t::flags_empty)
                    /* only actual matches are considered */
                    continue;
                if (i == key)
                    /* that's it! */
                    return this->swaths[s].to_value(d);
                i++;
            }
        }
        throw std::out_of_range("");
    }

    //const value_t operator[](size_t key) const { return this[key]; }
};



class bad_uservalue_cast
{
    std::string _text;   // оригинальная строка
    size_t _at;     // указывает, на каком символе ошибка

public:
    bad_uservalue_cast(const std::string &original, const size_t &at) noexcept {
        _text = original;
        _at = at;
    }
    
    // This declaration is not useless: http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
    ~bad_uservalue_cast() noexcept { }
    
    const std::string what() const noexcept {
        return ("bad_uservalue_cast(): \n"
                + _text + "\n"
                + std::string(_at, ' ') + "^" + std::string(_text.size() - _at - 1, '~'));
    };
    
    const std::string text() noexcept {
        return _text;
    };
    
    size_t at() const noexcept {
        return _at;
    };
};



class Scanner
{
public:
    /// Create file for storing matches
    RE::IProcess *handler;
    volatile bool stop_flag = false;
    volatile double scan_progress = 0;
    uintptr_t step = 1;
    
    explicit Scanner(IProcess *handler) {
        this->handler = handler;
    }
    
    void string_to_uservalue(const RE::Edata_type &data_type,
                             const std::string &text,
                             RE::Ematch_type *match_type,
                             RE::Cuservalue *vals);

    // FIXME[critical]: оно живое?
    // TODO[med]: modern snapshots, make scan_* snapshots-friendly (or make RE::handler snapshots friendly (or make father-class like RE::handler, and childs like RE::handler::process RE::handler::snapshot))
    // TODO[med]: first scan, next scan, next scan, scan compared to first scan, next scan, undo scan, etc...
    //bio::mapped_file make_snapshot(const std::string &path);

    /**
     *  This is used as first scan. It scans for values and allocates memory for each match.
     *
     *  Will read process memory and update 'matches_t'
     */
    bool scan_regions(RE::matches_t& writing_matches,
                      const RE::Edata_type& data_type,
                      const RE::Cuservalue *uservalue,
                      const RE::Ematch_type& match_type);

    //bool scan_snapshot(RE::matches_t& writing_matches,
    //                   const RE::Edata_type& data_type,
    //                   const RE::Cuservalue *uservalue,
    //                   const RE::Ematch_type& match_type);

    /**
     *  Update bytes of matches (it does not scan, it just updates value).
     *  Should be used after scan_regions().
     *
     *  Will read process memory and update 'matches_t'
     */
    bool scan_update(RE::matches_t& writing_matches);

    /**
     *  Remove flag for each byte if value mismatched.
     *  Should be used after scan_update().
     *
     *  Will update 'matches_t'
     */
    bool scan_recheck(RE::matches_t& writing_matches,
                      const RE::Edata_type& data_type,
                      const RE::Cuservalue *uservalue,
                      RE::Ematch_type match_type);

private:
    /**
     *  Usually used after scan_* methods
     */
    bool scan_fit(RE::matches_t& writing_matches);
};



class pointerscan
{
public:
    explicit pointerscan(IProcess *handler) {
        this->handler = handler;
        omp_init_lock(&writelock);
    }

    ~pointerscan() {
        omp_destroy_lock(&writelock);
    }

    void helper(vector<uintptr_t> off, uintptr_t last, uintptr_t address, pointer_swath& ps) {
        if (off.size() == max_level) {
            // cout<<"helper: reached maxlevel"<<endl; //fixme [debug] #4
            return;
        }

        if (address >= last-min_offset && address < last+max_offset) {
            vector<uintptr_t> offs = off;
            offs.push_back(address-(last-min_offset));
            // cout<<"helper: level"<<offs.size()<<": off: {"<<ps.file.filename()<<"+";
            // for (const auto& i: offs)
            //     cout << HEX(i) << ",";
            // cout<<"\b} FOUND"<<endl; //fixme [debug] #1
            omp_set_lock(&writelock);
            ps.offsets.push_back(offs); /// success
            omp_unset_lock(&writelock);
        }

        off.push_back(0);
        for (size_t o1 = min_offset; o1 < max_offset; o1 += align) {
            off.back() = o1;
            // cout<<"helper: level"<<off.size()<<": off: {"<<ps.file.filename()<<"+";
            // for (const auto& i: off)
            //     cout << HEX(i) << ",";
            // cout<<"\b}"<<endl; //fixme [debug] #1

            uintptr_t s_last;
            if (!handler->read(last+o1, &s_last)) {
                // cout<<"helper: cannot dereference"<<endl; //fixme [debug] #3
                continue;
            }

            /// for each offset in level2
            helper(off, s_last, address, ps);
        }
    }

    std::vector<pointer_swath>
    scan_regions(uintptr_t address)
    {
        using std::cout, std::endl;

        scan_progress = 0.0;

        std::vector<pointer_swath> ret;
        size_t static_regions_count = 0;
        size_t static_regions_bytes = 0;
        for (RE::Region& region : handler->regions) {
            if (region.r2) {
                static_regions_count++;
                static_regions_bytes += region.size;
            }
        }
        double scan_step = 1.0/static_regions_bytes;
        double scan_last_printed = 0.0;

        /// for each region
        size_t static_region_counter = 0;
        for (RE::Region& region : handler->regions) {
            /// if region is static
            if (!region.r2) {
                continue;
            }
            cout<<"scan_regions: region["<<static_region_counter++<<"/"<<static_regions_count<<"]: "<<region<<endl; //fixme [debug] #0

            pointer_swath ps;
            ps.file = region.file;
            // cout<<"region.file: "<<region.file<<endl; //fixme [debug] #0

            std::vector<RBinSection *> sections = region.r2.get_sections();
            uintptr_t static_offset_to = 0;
            for(RBinSection *s : sections) {
                uintptr_t se = s->vaddr + s->vsize;
                static_offset_to = std::max(static_offset_to, se);
            }

            // static_offset_to = 0x800f; //fixme [debug] REMOVEME

            uintptr_t static_begin = region.address;
            uintptr_t static_end = region.address + static_offset_to;

            // cout<<"static_begin: "<<region.file.filename()<<endl; //fixme [debug] #0
            // cout<<"static_end: "<<region.file.filename()<<"+"<<HEX(static_offset_to)<<endl; //fixme [debug] #0


            if (address >= static_begin && address < static_end) {
                std::vector<uintptr_t> off {address-static_begin};
                // cout<<"static: level"<<off.size()<<": off: {"<<ps.file.filename()<<"+";
                // for (const auto i: off)
                //     cout << HEX(i) << ",";
                // cout<<"\b} FOUND"<<endl; //fixme [debug] #1
                ps.offsets.push_back(off); /// success
            }

            /// for each level0 pointer
#pragma omp parallel for
            for (uintptr_t o = 0; o < static_offset_to; o += align) {
                std::vector<uintptr_t> off {o};
                off.reserve(max_level);
                // cout<<"static: level"<<off.size()<<": off: {"<<ps.file.filename()<<"+";
                // for (const auto& i: off)
                //     cout << HEX(i) << ",";
                // cout<<"\b}"<<endl; //fixme [debug] #1
                uintptr_t last = static_begin + o;
                if (!handler->read(last, &last)) {
                    // cout<<"static: cannot dereference"<<endl; //fixme [debug] #3
                    continue;
                }

                /// for each offset in level1
                helper(off, last, address, ps);

                // scan_progress+=scan_step;
                // if (scan_progress-scan_last_printed > 0.01) {
                //     scan_last_printed = scan_progress;
                //     cout<<scan_progress<<": [";
                //     for(size_t i = 0; i < 70*scan_progress-1; i++)
                //         cout<<"=";
                //     cout<<">";
                //     for(size_t i = 70.0*scan_progress-1; i < 70; i++)
                //         cout<<" ";
                //     cout<<"]"<<endl;
                // }
                // if (stop_flag)
                //     break;
                // if UNLIKELY(stop_flag)
                //     continue;
            }

            if (!ps.offsets.empty()) {
                ret.push_back(ps);
            }
            // break; //fixme [debug] REMOVEME
        }
        scan_progress = 1.0;
        return ret;
    }

public:
    /// Create file for storing matches
    RE::IProcess *handler;
    volatile bool stop_flag = false;
    volatile double scan_progress = 0.0;
    uintptr_t max_level = 3;

    uintptr_t min_offset = 0;
    uintptr_t max_offset = 0x100;
    uintptr_t align = sizeof(uint32_t);

    omp_lock_t writelock;
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


NAMESPACE_END(RE)
