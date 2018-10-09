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
#include <cstdlib>
//
#include "core.hh"
#include "value.hh"
#include "scanroutines.hh"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

class TestTimer
{
public:
    TestTimer(const std::string & name) : name(name),
                                          start(boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time())
    {
    }

    ~TestTimer()
    {
        using namespace std;
        using namespace boost;
        posix_time::ptime now(date_time::microsec_clock<posix_time::ptime>::local_time());
        posix_time::time_duration d = now - start;

        cout << fixed<<d.total_nanoseconds() / 1000000000.0 << " seconds "<< name << endl;
        usleep(1000);
    }

private:
    std::string name;
    boost::posix_time::ptime start;
};


namespace RE {

namespace bio = boost::iostreams;

using namespace std;
using namespace std::chrono;


class match_t {
public:
    uintptr_t address;
    union {
        mem64_t memory;
        uint8_t *bytes;
    };
    uint16_t flags;
    
    explicit match_t(uintptr_t address, uint16_t userflag = flags_empty) {
        this->address = address;
        this->flags = userflag;
    }

    explicit match_t(uintptr_t address, mem64_t memory, uint16_t userflag = flags_empty) {
        this->address = address;
        this->memory = memory;
        this->flags = userflag;
    }

    inline
    match_flags
    flag()
    {
        if      (this->flags & RE::flag_i64) return RE::flag_i64;
        else if (this->flags & RE::flag_i32) return RE::flag_i32;
        else if (this->flags & RE::flag_i16) return RE::flag_i16;
        else if (this->flags & RE::flag_i8)  return RE::flag_i8;
        else if (this->flags & RE::flag_u64) return RE::flag_u64;
        else if (this->flags & RE::flag_u32) return RE::flag_u32;
        else if (this->flags & RE::flag_u16) return RE::flag_u16;
        else if (this->flags & RE::flag_u8)  return RE::flag_u8;
        else if (this->flags & RE::flag_f64) return RE::flag_f64;
        else if (this->flags & RE::flag_f32) return RE::flag_f32;
        return RE::flags_empty;
    }

    inline
    std::string
    flag2str()
    {
        if      (this->flags & RE::flag_i64) return "i64";
        else if (this->flags & RE::flag_i32) return "i32";
        else if (this->flags & RE::flag_i16) return "i16";
        else if (this->flags & RE::flag_i8)  return "i8";
        else if (this->flags & RE::flag_u64) return "u64";
        else if (this->flags & RE::flag_u32) return "u32";
        else if (this->flags & RE::flag_u16) return "u16";
        else if (this->flags & RE::flag_u8)  return "u8";
        else if (this->flags & RE::flag_f64) return "f64";
        else if (this->flags & RE::flag_f32) return "f32";
        return "";
    }

    inline
    std::string
    val2str()
    {
        if      (this->flags & RE::flag_i64) return std::to_string(this->memory.i64);
        else if (this->flags & RE::flag_i32) return std::to_string(this->memory.i32);
        else if (this->flags & RE::flag_i16) return std::to_string(this->memory.i16);
        else if (this->flags & RE::flag_i8)  return std::to_string(this->memory.i8);
        else if (this->flags & RE::flag_u64) return std::to_string(this->memory.u64);
        else if (this->flags & RE::flag_u32) return std::to_string(this->memory.u32);
        else if (this->flags & RE::flag_u16) return std::to_string(this->memory.u16);
        else if (this->flags & RE::flag_u8)  return std::to_string(this->memory.u8);
        else if (this->flags & RE::flag_f64) return std::to_string(this->memory.f64);
        else if (this->flags & RE::flag_f32) return std::to_string(this->memory.f32);
        return "";
    }

    inline
    std::string
    address2str()
    {
        char *address_string;
        const uint8_t *b = reinterpret_cast<const uint8_t *>(&this->address);
        asprintf(&address_string, /*0x*/"%02x%02x%02x%02x%02x%02x", b[5], b[4], b[3], b[2], b[1], b[0]);
        return string(address_string);
    }
};


/* Single match struct */
#pragma pack(push, 1)
struct byte_with_flags {
    uint8_t byte;
    match_flags flags;
};
#pragma pack(pop)


class swath_t
{
public:
    uintptr_t base_address;
    size_t data_allocated;
    size_t data_count;
    byte_with_flags *data;

    FORCE_INLINE
    void
    constructor(uintptr_t base_address) {
        this->base_address = base_address;
        this->data_allocated = 1;
        this->data_count = 0;
        this->data = NULL;
        if ((data = (byte_with_flags *) realloc(data, data_allocated * sizeof(byte_with_flags))) == nullptr) {
            puts("ERROR: constructor(): not allocated");
            return;
        }
    }


    FORCE_INLINE
    void
    allocate_enough(size_t count) {
        if LIKELY(count >= data_allocated) {
            while(count >= data_allocated)
                data_allocated *= 2;
            if ((data = (byte_with_flags *) realloc(data, data_allocated*sizeof(byte_with_flags))) == nullptr) {
                puts("allocate_enough(): not allocated");
                return;
            }
        }
    }

    FORCE_INLINE
    void
    append(byte_with_flags d) {
        if UNLIKELY(data_count >= data_allocated) {
            allocate_enough(data_count);
        }
        data_count++;
        data[data_count-1] = d;
    }

    FORCE_INLINE
    uintptr_t
    remote_get(size_t n) {
        return base_address + static_cast<uintptr_t>(n);
    }

    FORCE_INLINE
    uintptr_t
    remote_back() {
        return remote_get(this->data_count - 1);
    }


/* for printable text representation */
//    inline
//    void data_to_printable_string(char *buf,
//                                  int buf_length,
//                                  size_t index,
//                                  int string_length)
//    {
//        long swath_length = this->data_size - index;
//        /* TODO: what if length is too large ? */
//        long max_length = (swath_length >= string_length) ? string_length : swath_length;
//        int i;
//
//        for(i = 0; i < max_length; i++) {
//            uint8_t byte = this->data[index + i].byte;
//            buf[i] = isprint(byte) ? byte : '.';
//        }
//        buf[i] = 0; /* null-terminate */
//    }


/* for bytearray representation */
//    inline
//    void data_to_bytearray_text(char *buf,
//                                int buf_length,
//                                size_t index,
//                                int bytearray_length)
//    {
//        int i;
//        int bytes_used = 0;
//        long swath_length = this->data_size - index;
//
//        /* TODO: what if length is too large ? */
//        long max_length = (swath_length >= bytearray_length) ?
//                          bytearray_length : swath_length;
//
//        for(i = 0; i < max_length; i++) {
//            uint8_t byte = this->data[index + i].byte;
//
//            /* TODO: check error here */
//            snprintf(buf + bytes_used, buf_length - bytes_used,
//                     (i < max_length - 1) ? "%02x " : "%02x", byte);
//            bytes_used += 3;
//        }
//    }


/* only at most sizeof(int64_t) bytes will be read,
   if more bytes are needed (e.g. bytearray),
   read them separately (for performance) */
#ifdef call_always_
    FORCE_INLINE
#endif
#ifdef call_noinline_
    NEVER_INLINE
#endif
    value_t
    data_to_val_aux(size_t index, size_t swath_length)
    {
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

        /* Unrolling this will improve performance by 40% for gcc.
         * TODO to investigate */
#if RE_ADJUST_DATA_TO_VAL_LOOP == 1
# if RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL == 0
        #pragma nounroll
# elif RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL > 0
        #pragma unroll(RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL)
#endif
        for(uint8_t i = 0; i < max_bytes; i++) {
            /* Both uint8_t, no explicit casting needed */
            val.bytes[i] = data[index + i].byte;
        }
#elif RE_ADJUST_DATA_TO_VAL_LOOP == 2
        if (max_bytes > 0) { val.bytes[0] = data[index + 0].byte;
        if (max_bytes > 1) { val.bytes[1] = data[index + 1].byte;
        if (max_bytes > 2) { val.bytes[2] = data[index + 2].byte;
        if (max_bytes > 3) { val.bytes[3] = data[index + 3].byte;
        if (max_bytes > 4) { val.bytes[4] = data[index + 4].byte;
        if (max_bytes > 5) { val.bytes[5] = data[index + 5].byte;
        if (max_bytes > 6) { val.bytes[6] = data[index + 6].byte;
        if (max_bytes > 7) { val.bytes[7] = data[index + 7].byte;
        }}}}}}}}
#elif RE_ADJUST_DATA_TO_VAL_LOOP == 3
        switch (max_bytes) {
            case 8:
                val.bytes[7] = data[index + 7].byte;
            case 7:
                val.bytes[6] = data[index + 6].byte;
            case 6:
                val.bytes[5] = data[index + 5].byte;
            case 5:
                val.bytes[4] = data[index + 4].byte;
            case 4:
                val.bytes[3] = data[index + 3].byte;
            case 3:
                val.bytes[2] = data[index + 2].byte;
            case 2:
                val.bytes[1] = data[index + 1].byte;
            case 1:
                val.bytes[0] = data[index + 0].byte;
        }
#elif RE_ADJUST_DATA_TO_VAL_LOOP == 4
        switch (max_bytes) {
            case 1:
                val.bytes[0] = data[index + 0].byte;
                break;
            case 2:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                break;
            case 3:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                break;
            case 4:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                val.bytes[3] = data[index + 3].byte;
                break;
            case 5:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                val.bytes[3] = data[index + 3].byte;
                val.bytes[4] = data[index + 4].byte;
                break;
            case 6:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                val.bytes[3] = data[index + 3].byte;
                val.bytes[4] = data[index + 4].byte;
                val.bytes[5] = data[index + 5].byte;
                break;
            case 7:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                val.bytes[3] = data[index + 3].byte;
                val.bytes[4] = data[index + 4].byte;
                val.bytes[5] = data[index + 5].byte;
                val.bytes[6] = data[index + 6].byte;
                break;
            case 8:
                val.bytes[0] = data[index + 0].byte;
                val.bytes[1] = data[index + 1].byte;
                val.bytes[2] = data[index + 2].byte;
                val.bytes[3] = data[index + 3].byte;
                val.bytes[4] = data[index + 4].byte;
                val.bytes[5] = data[index + 5].byte;
                val.bytes[6] = data[index + 6].byte;
                val.bytes[7] = data[index + 7].byte;
                break;
        }
#endif
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
        swaths_allocated = 1;
        swaths_count = 0;
        if ((swaths = (swath_t *) realloc(swaths, swaths_allocated*sizeof(swath_t))) == nullptr) {
            puts("matches_t(): not allocated");
            return;
        }
#if RE_ADJUST_INIT_FIRST_SWATH == 1
        // Scan 1/3 done in: 2.23431 seconds
        // Scan 2/3 done in: 4.66321 seconds
        swaths_count = 1;
        swaths[swaths_count-1].constructor(0);
#endif
    }

    ~matches_t() {
        clog<<"~matches_t()"<<endl;
        free(swaths);
    }

    FORCE_INLINE
    void
    add_element(const uintptr_t& remote_address, const mem64_t *remote_memory, const match_flags& flags)
    {
#if RE_ADJUST_INIT_FIRST_SWATH == 2
        // Scan 1/3 done in: 2.23431 seconds
        // Scan 2/3 done in: 4.66321 seconds
        if UNLIKELY(swaths_count == 0) {
            swaths_count++;
            swaths[swaths_count-1].constructor(remote_address);
        }
#endif
        size_t remote_delta = remote_address - swaths[swaths_count-1].remote_back();
        size_t local_delta = remote_delta * sizeof(byte_with_flags);

        if (local_delta >= sizeof(swath_t) + sizeof(byte_with_flags)) {
            // It is more memory-efficient to start a new swath
//            clog<<"It is more memory-efficient to start a new swath"<<endl;
            swaths_count++;
            if UNLIKELY(swaths_count > swaths_allocated) {
                swaths_allocated *= 2;
//                clog<<"swaths_count: "<<swaths_count<<endl;
//                clog<<"swaths_allocated: "<<swaths_allocated<<endl;
                if ((swaths = (swath_t *) realloc(swaths, swaths_allocated*sizeof(swath_t))) == nullptr) {
                    puts("add_element(): not allocated");
                    return;
                }
            }
            swaths[swaths_count-1].constructor(remote_address);
        } else {
            // It is more memory-efficient to write over the intervening space with null values
#if RE_ADJUST_NULL_EMPLACE==1 // primitive
            // 538229829
            // Scan 1/3 done in: 2.23351 seconds
            // Scan 2/3 done in: 4.70711 seconds
            while (remote_delta-->1)
                swaths[swaths_count-1].append(byte_with_flags{ 0, flags_empty });
#elif RE_ADJUST_NULL_EMPLACE==2 // from scanmem (truncated)
            // 538229829
            // Scan 1/3 done in: 2.67642 seconds
            // Scan 2/3 done in: 5.13039 seconds
            swaths[swaths_count-1].allocate_enough(swaths[swaths_count-1].data_count+remote_delta-1);
            memset(swaths[swaths_count-1].data + swaths[swaths_count-1].data_count,
                    0,
                    local_delta - sizeof(byte_with_flags));
            swaths[swaths_count-1].data_count += remote_delta-1;
#elif RE_ADJUST_NULL_EMPLACE==3 // from scanmem
            // 538229829
            // Scan 1/3 done in: 2.67642 seconds
            // Scan 2/3 done in: 5.13039 seconds
            swaths[swaths_count-1].allocate_enough(swaths[swaths_count-1].data_count+remote_delta-1);
            switch (local_delta) {
                case 1:
                    /* do nothing, the new value is right after the old */
                    break;
                case 2:
                    memset(swaths[swaths_count - 1].data + swaths[swaths_count - 1].data_count,
                           0,
                           sizeof(byte_with_flags));
                    break;
                default:
                    memset(swaths[swaths_count - 1].data + swaths[swaths_count - 1].data_count,
                           0,
                           local_delta - sizeof(byte_with_flags));
            }
            swaths[swaths_count-1].data_count += remote_delta-1;
#endif
        }
//        clog<<"add me: "<<swaths_count<<endl;
        swaths[swaths_count-1].append(byte_with_flags{remote_memory->u8, flags});
    }

    size_t
    mem_allo()
    {
        size_t size = 0;
        for(size_t s = 0; s < swaths_count; s++)
            size += swaths[s].data_allocated;
        return size+swaths_allocated*(sizeof(swath_t));
    }

    size_t
    mem_virt()
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
    nth_match(size_t n) {
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
        return match_location{nullptr, 0};
    }

    // todo[critical]: must return match_t
    // todo[high]: must return full string, not just 8 bytes
    // todo[med]: must return match with old value
    // handlers.c:375
    match_t
    get(size_t n, Edata_type dt)
    {
        match_t ret{0, RE::flags_empty};
        match_location m = nth_match(n);
        if (m.swath) {
            ret.flags = m.swath->data[m.index].flags;
            switch(dt)
            {
                case Edata_type::BYTEARRAY:
//                    break;
                case Edata_type::STRING:
//                    break;
                default: /* numbers */
//                    RE::value_t val = m.swath->data_to_val(m.index);
                    for(size_t i = 0; i < RE::flags_to_memlength(dt, ret.flags); i++) {
                        /* Both uint8_t, no explicit casting needed */
                        ret.bytes[i] = m.swath->data[m.index + i].byte;
                    }
            }
        }
        return ret;
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
//
    

    std::vector<Cregion>  regions;
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

        Cregion region;
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






//class matches_t2 {
//public:
//    size_t matches_size = 0;
//    std::vector<swath_t> swaths;
//
//    matches_t2() {
//        clog<<"matches_t()"<<endl;
//    }
//
//    ~matches_t2() {
//        clog<<"~matches_t()"<<endl;
//    }
//
//    // fixme[low]: slow, because 'new' calls constructor
//    void
//    inline
////    __attribute__((always_inline))
//    add_element(const uintptr_t& remote_address, const RE::mem64_t *remote_memory, const RE::match_flags& flags) {
//        if UNLIKELY(swaths.empty()) {
//            swaths.emplace_back(remote_address);
//        } else {
//            size_t remote_delta = remote_address - swaths.back().remote_back();
//            size_t local_delta = remote_delta*sizeof(byte_with_flags);
//            if UNLIKELY(local_delta > sizeof(swath_t) + sizeof(byte_with_flags)) {
//                // It is more memory-efficient to start a new swath
//                swaths.emplace_back(remote_address);
//            } else {
//                // It is more memory-efficient to write over the intervening space with null values
//                swaths.back().data.resize(swaths.back().data.size() + remote_delta - 1);
//            }
//        }
//        swaths.back().data.push_back(byte_with_flags{remote_memory->u8, flags});
//    }
//
//    size_t
//    mem_allo()
//    {
//        size_t size = 0;
//        for(auto& swath : swaths)
//            size += swath.data.capacity()*sizeof(byte_with_flags)
//                    + sizeof(swath.data) + sizeof(swaths.back().data); // NOLINT(bugprone-sizeof-container)
//        return size + swaths.capacity()*sizeof(swath_t) + sizeof(swaths); // NOLINT(bugprone-sizeof-container)
//    }
//
//    size_t
//    mem_virt()
//    {
//        size_t size = 0;
//        for(auto& swath : swaths)
//            size += swath.data.size()*sizeof(byte_with_flags);
//        return size + swaths.size()*sizeof(swath_t);
//    }
//
//    size_t
//    size()
//    {
//        size_t size = 0;
//        for(auto& swath : swaths)
//            for(auto& d : swath.data)
//                if (d.flags > flags_empty)
//                    size += 1;
//        return size;
//    }
//
//    match_location
//    nth_match(size_t n) {
//        size_t i = 0;
//        for(auto& swath : swaths) {
//            /* only actual matches are considered */
//            size_t i_inside = 0;
//            for(auto& d : swath.data) {
//                if (d.flags > flags_empty) {
//                    if(i == n)
//                        return match_location{&swaths[i], i_inside};
//                    i++;
//                }
//                i_inside++;
//            }
//        }
//
//        /* invalid match id */
//        return match_location{nullptr, 0};
//    }
//
//    // todo[critical]: must return match_t
//    // todo[high]: must return full string, not just 8 bytes
//    // todo[med]: must return match with old value
//    // handlers.c:375
//    match_t
//    get(size_t n, Edata_type dt)
//    {
//        match_t ret{0, RE::flags_empty};
//        match_location m = nth_match(n);
//        if (m.swath) {
//            ret.flags = m.swath->data[m.index].flags;
//            switch(dt)
//            {
//                case Edata_type::BYTEARRAY:
////                    break;
//                case Edata_type::STRING:
////                    break;
//                default: /* numbers */
//                    RE::value_t val = m.swath->data_to_val(m.index);
//                    for(size_t i = 0; i < RE::flags_to_memlength(dt, ret.flags); i++) {
//                        /* Both uint8_t, no explicit casting needed */
//                        ret.bytes[i] = m.swath->data[m.index + i].byte;
//                    }
//            }
//        }
//        return ret;
//    }
//
//
//
//    /* deletes matches in [start, end) and resizes the matches array */
//    //todo resolve this
////    void
////    delete_in_address_range(unsigned long *num_matches,
////                            uintptr_t start_address,
////                            uintptr_t end_address)
////    {
////        size_t reading_iterator = 0;
////        swath_t *reading_swath_index = this->swaths;
////
////        swath_t reading_swath = *reading_swath_index;
////
////        swath_t *writing_swath_index = this->swaths;
////
////        writing_swath_index->base_address = 0;
////        writing_swath_index->data_count = 0;
////
////        *num_matches = 0;
////
////        while (reading_swath.base_address) {
////            uintptr_t address = reading_swath.base_address + reading_iterator;
////
////            if (address < start_address || address >= end_address) {
////                byte_with_flag old_byte;
////
////                old_byte = reading_swath_index->data[reading_iterator];
////
////                /* Still a candidate. Write data.
////                    (We can get away with overwriting in the same array because
////                     it is guaranteed to take up the same number of bytes or fewer,
////                     and because we copied out the reading swath metadata already.)
////                    (We can get away with assuming that the pointers will stay
////                     valid, because as we never add more data to the array than
////                     there was before, it will not reallocate.) */
//////                writing_swath_index = this->add_element(writing_swath_index, //TODO 12312312
//////                                                        address,
//////                                                        old_byte.old_value,
//////                                                        old_byte.flags);
////
////                /* actual matches are recorded */
////                if (old_byte.flags != flags_empty)
////                    (*num_matches)++;
////            }
////
////            /* go on to the next one... */
////            reading_iterator++;
////            if (reading_iterator >= reading_swath.data_count) {
////
////                reading_swath_index = (swath_t *)
////                        (&reading_swath_index->data[reading_swath.data_count]);
////
////                reading_swath = *reading_swath_index;
////
////                reading_iterator = 0;
////            }
////        }
////
////        this->null_terminate(writing_swath_index);
////    }
////
//
//
//    std::vector<Cregion>  regions;
//    std::string           path;
//    bio::mapped_file      snapshot_mf;
//    bio::mapped_file      flags_mf;
//
//
//    explicit matches_t2(const bio::mapped_file& snapshot_mapped_file,
//                       const bio::mapped_file& flags_mapped_file)
//    {
//        snapshot_mf = snapshot_mapped_file;
//        flags_mf = flags_mapped_file;
//
//        char *snapshot_begin = snapshot_mf.data();
//        char *snapshot_end = snapshot_begin + snapshot_mf.size();
//        char *snapshot = snapshot_begin;
//
//        Cregion region;
//        while(snapshot < snapshot_end) {
//            memcpy(&region,
//                   snapshot,
//                   2*sizeof(region.address));
//            if (region.size <= 1) break;
//            snapshot += region.size;
//            regions.emplace_back(region);
//        }
//    }
//
//    size_t size(bool a) {
//        char *flags_begin = flags_mf.data();
//        char *flags_end = flags_begin + flags_mf.size();
//        char *flags = flags_begin;
//
//        size_t size = 0;
//        uint16_t flag;
//
//        while (flags < flags_end) {
//            memcpy(&flag,
//                   flags,
//                   sizeof(uint16_t));
//            if (flag)
//                size++;
//            flags += sizeof(uint16_t);
//        }
//
//        return size;
//    }
//
//    bool operator !() const {
//        return !snapshot_mf || !flags_mf;
//    }
//};



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
    RE::Handle *handle;
    bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
    
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    void string_to_uservalue(const RE::Edata_type &data_type,
                             const std::string &text,
                             RE::Ematch_type *match_type,
                             RE::Cuservalue *vals);
    
    bio::mapped_file make_snapshot(const std::string &path);
    
    bool scan(RE::matches_t& matches_sink,
              const RE::Edata_type& data_type,
              const RE::Cuservalue *uservalue,
              const RE::Ematch_type& match_type);

    bool scan_next(RE::matches_t& matches_source,
                   RE::matches_t& matches_sink,
                   const RE::Edata_type& data_type,
                   const RE::Cuservalue *uservalue,
                   RE::Ematch_type match_type);
    
    bool scan_reset();
};

} //namespace RE

#endif //RE_SCANNER_HH
