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
#include "core.hh"
#include "value.hh"

namespace bio = boost::iostreams;


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
struct byte_with_flags
{
    uint8_t byte;
    uint16_t flags;
};
#pragma pack(pop)


class swath_t
{
public:
    uintptr_t base_address;
    std::vector<byte_with_flags> data;
    
    explicit swath_t(uintptr_t base_address)
            : base_address(base_address) {
        data.reserve(8);
    }
    
    uintptr_t
    remote_get(size_t n) {
        return base_address + n;
    }
    
    uintptr_t
    remote_last() {
        return remote_get(data.size() - 1);
    }
    
    size_t
    size() {
        return sizeof(*this)+sizeof(data)+8*sizeof(byte_with_flags);
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
    size_t max_needed_bytes;
    size_t bytes_allocated;
    size_t matches_size = 0;
    std::vector<swath_t> swaths;
    
    
    // fixme: very slow
    void
    add_element(const uintptr_t& address, const mem64_t *memory, const uint16_t& flags)
    {
        if (swaths.size() == 0)
            swaths.push_back(swath_t(address));
        size_t remote_delta = address - swaths.back().remote_last();
        size_t local_delta = remote_delta * sizeof(byte_with_flags);
        
        if (local_delta >= sizeof(swath_t) + sizeof(swaths) + 8 * sizeof(byte_with_flags))
            swaths.emplace_back(swath_t(address));
        else
            while (remote_delta-->0)  // tends to zero xD
                swaths.back().data.push_back(byte_with_flags{ 0, 0 });
        swaths.back().data.push_back(byte_with_flags{memory->u8, flags});
    }
    
    
    match_location
    nth_match(size_t n)
    {
        size_t i = 0;
        for(swath_t& swath : this->swaths) {
            /* only actual matches are considered */
            size_t i_inside = 0;
            for(const byte_with_flags& bof : swath.data) {
                if (bof.flags > flags_empty) {
                    if (i == n)
                        return match_location {&swath, i_inside};
                    i++;
                }
                i_inside++;
            }
        }
        
        /* invalid match id */
        return (match_location) { 0, 0 };
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
//        writing_swath_index->number_of_bytes = 0;
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
//            if (reading_iterator >= reading_swath.number_of_bytes) {
//                
//                reading_swath_index = (swath_t *)
//                        (&reading_swath_index->data[reading_swath.number_of_bytes]);
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
    
    matches_t () {};
    
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
    
    bool operator !() const {
        return !snapshot_mf || !flags_mf;
    }
    
    size_t size() {
        if (!*this) {
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
        } else {
            
        }
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
                   scan_match_type_t match_type,
                   const uservalue_t *uservalue);
    
    bool scan_reset();
    
};

#endif //RE_SCANNER_HH
