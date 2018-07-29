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

#ifndef RE_SCANNER_HH
#define RE_SCANNER_HH

#include <cinttypes>
#include <memory>
#include <cmath>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <chrono>
#include <deque>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>
#include <stdlib.h>
//
#include "core.hh"
#include "value.hh"
#include "scanroutines.hh"

namespace bio = boost::iostreams;

// todo[med]: remove using
using namespace std;
using namespace std::chrono;


/**
 * return the smallest power of two value
 * greater than x
 */
__attribute__ ((const))
static inline uint64_t p2(uint64_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

class match_t {
public:
    uintptr_t address;
    mem64_t memory;
    mem64_t memory_old;
    uint16_t flags;
    
    match_t() {
        this->address = 0;
        this->flags = 0;
    }
    
    explicit match_t(uintptr_t address, mem64_t memory, uint16_t userflag = flags_empty) {
        this->address = address;
        this->memory = memory;
        this->flags = userflag;
    }
};


/* Single match struct */
#pragma pack(push, 1)
struct byte_with_flags {
    uint8_t byte;
    uint16_t flags;
};
#pragma pack(pop)


class swath_t
{
public:
    uintptr_t base_address;
    size_t data_allocated;
    size_t data_count;
    byte_with_flags *data;

    swath_t() {
        data_allocated = 0;
        data_count = 0;
        data = NULL;
    }

    // fixme[low]: slow, because 'new' calls constructor and copies data
    void
    allocate_enough(size_t count) {
        if (LIKELY(count >= data_allocated)) {
            if (UNLIKELY(data_allocated == 0))
                data_allocated = 1;
            while(count >= data_allocated)
                data_allocated *= 2;
            byte_with_flags *newarray = new byte_with_flags[data_allocated];
//            byte_with_flags *newarray = (byte_with_flags *) realloc(data, data_allocated*sizeof(byte_with_flags));
            std::copy(data, data + std::min(data_count, data_allocated), newarray);
            delete[] data;
            data = newarray;
        }
    }

    void
    append(byte_with_flags d) {
        if (UNLIKELY(data_count >= data_allocated)) {
            allocate_enough(data_count);
        }
        data[data_count] = d;
        data_count++;
    }
    
    uintptr_t
    remote_get(size_t n) {
        return base_address + n;
    }
    
    uintptr_t
    remote_back() {
        return remote_get(this->data_count - 1);
    }
    
    inline
    value_t
    data_to_val_aux(size_t index, size_t swath_length) const
    {
        unsigned int i;
        value_t val;
        size_t max_bytes = swath_length - index;
        
        /* Init all possible flags in a single go.
         * Also init length to the maximum possible value */
        val.flags = flags_max;
        
        /* NOTE: This does the right thing for VLT because the flags are in
         * the same order as the number representation (for both endians), so
         * that the zeroing of a flag does not change useful bits of `length`. */
        if (max_bytes > 8)
            max_bytes = 8;
        if (max_bytes < 8) val.flags &= ~flags_64b;
        if (max_bytes < 4) val.flags &= ~flags_32b;
        if (max_bytes < 2) val.flags &= ~flags_16b;
        if (max_bytes < 1) val.flags = flags_empty;
        
        for(i = 0; i < max_bytes; i++) {
            /* Both uint8_t, no explicit casting needed */
            val.bytes[i] = data[index + i].byte;
        }
        
        /* Truncate to the old flags, which are stored with the first matched byte */
        val.flags &= data[index].flags;
        
        return val;
    }
};


/* Location of a match in a matches_t */
struct match_location
{
    swath_t *swath;
    size_t index;
};


class matches_t {
public:
    size_t matches_size = 0;
//    std::vector<swath_t> swaths;
    size_t swaths_allocated = 0;
    size_t swaths_count = 0;
    swath_t *swaths = nullptr;

    matches_t() {
        clog<<"matches_t()"<<endl;
        swaths = new swath_t[1];
        swaths_allocated = 1;
    }

    ~matches_t() {
        clog<<"~matches_t()"<<endl;
        delete[] swaths;
    }

    // fixme[low]: slow, because 'new' calls constructor
    void
    add_element(const uintptr_t& remote_address, const mem64_t *remote_memory, const uint16_t& flags)
    {
        size_t remote_delta = remote_address - swaths[swaths_count-1].remote_back();
        size_t local_delta = remote_delta * sizeof(byte_with_flags);

        if (UNLIKELY(local_delta >= sizeof(swath_t) + sizeof(byte_with_flags))) {
            // It is more memory-efficient to start a new swath
            if (UNLIKELY(swaths_count >= swaths_allocated)) {
                swaths_allocated *= 2;
                swath_t *newarray = new swath_t[swaths_allocated];
                std::copy(swaths, swaths + std::min(swaths_count, swaths_allocated), newarray);
                delete[] swaths;
                swaths = newarray;
            }
            swaths[swaths_count].base_address = remote_address;
            swaths_count++;
        } else {
            // It is more memory-efficient to write over the intervening space with null values
            swaths[swaths_count-1].data_count += remote_delta - 1;
        }
        swaths[swaths_count-1].append(byte_with_flags{remote_memory->u8, flags});
    }


    size_t
    allo()
    {
        size_t size = 0;
        for(size_t s = 0; s < swaths_count; s++)
            size += swaths[s].data_allocated;
        return size+swaths_allocated*(sizeof(swath_t));
    }

    size_t
    virt()
    {
        size_t size = 0;
        for(size_t s = 0; s < swaths_count; s++)
            size += swaths[s].data_count;
        return size+swaths_count*(sizeof(swath_t));
    }
    
    size_t
    size()
    {
        size_t size = 0;
        for(size_t s = 0; s < swaths_count; s++) {
            for(size_t d = 0; d < swaths[s].data_count; d++)
                if (swaths[s].data[d].flags > flags_empty)
                    size += 1;
        }
        return size;
    }
    
    
    match_location
    nth_match(size_t n)
    {
        size_t i = 0;
        for(size_t s = 0; s < swaths_count; s++) {
            /* only actual matches are considered */
            size_t i_inside = 0;
            for(size_t d = 0; d < swaths[s].data_count; d++) {
                if (swaths[s].data[d].flags > flags_empty) {
                    if (i == n)
                        return match_location {&swaths[i], i_inside};
                    i++;
                }
                i_inside++;
            }
        }
        
        /* invalid match id */
        return (match_location) { 0, 0 };
    }

    // todo[critical]: return match_t
    match_t
    get(size_t n)
    {
        match_location m = nth_match(n);
        if (m.swath)
            return m.swath->data[m.index];
        return
    }
    
    
    
    /* deletes matches in [start, end) and resizes the matches array */
    //todo resolve this
//    void
//    delete_in_address_range(unsigned long *num_matches,
//                            uintptr_t start_address,
//                            uintptr_t end_address)
//    {
//        size_t reading_iterator = 0;
//        swath_t *reading_swath_index = this->swaths;
//        
//        swath_t reading_swath = *reading_swath_index;
//        
//        swath_t *writing_swath_index = this->swaths;
//        
//        writing_swath_index->base_address = 0;
//        writing_swath_index->data_count = 0;
//        
//        *num_matches = 0;
//        
//        while (reading_swath.base_address) {
//            uintptr_t address = reading_swath.base_address + reading_iterator;
//            
//            if (address < start_address || address >= end_address) {
//                byte_with_flag old_byte;
//                
//                old_byte = reading_swath_index->data[reading_iterator];
//                
//                /* Still a candidate. Write data.
//                    (We can get away with overwriting in the same array because
//                     it is guaranteed to take up the same number of bytes or fewer,
//                     and because we copied out the reading swath metadata already.)
//                    (We can get away with assuming that the pointers will stay
//                     valid, because as we never add more data to the array than
//                     there was before, it will not reallocate.) */
////                writing_swath_index = this->add_element(writing_swath_index, //TODO 12312312
////                                                        address,
////                                                        old_byte.old_value,
////                                                        old_byte.flags);
//                
//                /* actual matches are recorded */
//                if (old_byte.flags != flags_empty)
//                    (*num_matches)++;
//            }
//            
//            /* go on to the next one... */
//            reading_iterator++;
//            if (reading_iterator >= reading_swath.data_count) {
//                
//                reading_swath_index = (swath_t *)
//                        (&reading_swath_index->data[reading_swath.data_count]);
//                
//                reading_swath = *reading_swath_index;
//                
//                reading_iterator = 0;
//            }
//        }
//        
//        this->null_terminate(writing_swath_index);
//    }
    
    
    
    std::vector<region_t> regions;
    std::string           path;
    bio::mapped_file      snapshot_mf;
    bio::mapped_file      flags_mf;


    explicit matches_t(const bio::mapped_file& snapshot_mapped_file,
                       const bio::mapped_file& flags_mapped_file)
    {
        snapshot_mf = snapshot_mapped_file;
        flags_mf = flags_mapped_file;
    
        char *snapshot_begin = snapshot_mf.data();
        char *snapshot_end = snapshot_begin + snapshot_mf.size();
        char *snapshot = snapshot_begin;
        
        region_t region;
        while(snapshot < snapshot_end) {
            memcpy(&region,
                   snapshot,
                   2*sizeof(region.address));
            if (region.size <= 1) break;
            snapshot += region.size;
            regions.emplace_back(region);
        }
    }
    
    size_t size(bool a) {
        char *flags_begin = flags_mf.data();
        char *flags_end = flags_begin + flags_mf.size();
        char *flags = flags_begin;
        
        size_t size = 0;
        uint16_t flag;
        
        while (flags < flags_end) {
            memcpy(&flag,
                   flags,
                   sizeof(uint16_t));
            if (flag)
                size++;
            flags += sizeof(uint16_t);
        }
        
        return size;
    }
    
    bool operator !() const {
        return !snapshot_mf || !flags_mf;
    }
};



class bad_uservalue_cast
{
    std::string _text;   // оригинальная строка
    size_t _at;     // указывает, на каком символе ошибка

public:
    bad_uservalue_cast(const std::string &original, const size_t &at) _GLIBCXX_USE_NOEXCEPT {
        _text = original;
        _at = at;
    }
    
    // This declaration is not useless: http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
    ~bad_uservalue_cast() _GLIBCXX_USE_NOEXCEPT { }
    
    const std::string what() const _GLIBCXX_USE_NOEXCEPT {
        return ("bad_uservalue_cast(): \n"
                + _text + "\n"
                + std::string(_at, ' ') + "^" + std::string(_text.size() - _at - 1, '~'));
    };
    
    const std::string text() _GLIBCXX_USE_NOEXCEPT {
        return _text;
    };
    
    size_t at() const _GLIBCXX_USE_NOEXCEPT {
        return _at;
    };
};



class Scanner
{
public:
    /// Create file for storing matches
    Handle *handle;
    volatile bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
    
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    void string_to_uservalue(const scan_data_type_t &data_type,
                             const std::string &text,
                             scan_match_type_t *match_type,
                             uservalue_t *vals);
    
    bio::mapped_file make_snapshot(const std::string &path);
    
    bool scan(matches_t& matches_sink,
              const scan_data_type_t& data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t& match_type);

    bool scan_next(const matches_t& matches_source,
                   matches_t& matches_sink,
                   const scan_data_type_t& data_type,
                   const uservalue_t *uservalue,
                   scan_match_type_t match_type);
    
    bool scan_reset();
    
};

#endif //RE_SCANNER_HH
