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


namespace bio = boost::iostreams;

namespace RE {


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
        return std::string(address_string);
    }
};


/* Single match struct */
#pragma pack(push, 1)
struct byte_with_flags {
    uint8_t byte;
    match_flags flags;
};
#pragma pack(pop)

using std::cout, std::endl;

class swath_t
{
public:
    uintptr_t base_address;
    std::vector<uint8_t > bytes;
    std::vector<match_flags> flags;

    swath_t(uintptr_t base_address) {
        this->base_address = base_address;
    }

    FORCE_INLINE
    void
    append(uint8_t byte, match_flags flag) {
        bytes.emplace_back(byte);
        flags.emplace_back(flag);
    }

    FORCE_INLINE
    uintptr_t
    remote_get(size_t n) {
        return base_address + static_cast<uintptr_t>(n);
    }

    FORCE_INLINE
    uintptr_t
    remote_back() {
        return remote_get(bytes.size() - 1);
    }

/* only at most sizeof(int64_t) bytes will be read,
   if more bytes are needed (e.g. bytearray),
   read them separately (for performance) */
#if RE_ADJUST_DATA_TO_VAL_INLINE == 0
    NEVER_INLINE
#elif RE_ADJUST_DATA_TO_VAL_INLINE == 1
    FORCE_INLINE
#endif
    value_t
    data_to_val_aux(size_t index)
    {
        value_t val;

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


/* Location of a match in a matches_t */
struct match_location
{
    swath_t *swath;
    size_t index;
};


class matches_t {
public:
    [[deprecated("To be removed")]]
    size_t matches_size = 0;
    std::vector<swath_t> swaths;

    matches_t() {
        std::clog<<"matches_t()"<<std::endl;
    }

    ~matches_t() {
        std::clog<<"~matches_t()"<<std::endl;
    }

    FORCE_INLINE
    void
    add_element(const uintptr_t& remote_address, const mem64_t *remote_memory, const match_flags& flags)
    {
        if UNLIKELY(swaths.empty()) {
            swaths.emplace_back(remote_address);
        }

        size_t remote_delta = remote_address - swaths.back().remote_back();
        size_t local_delta = remote_delta * sizeof(byte_with_flags);

        if (local_delta >= sizeof(swath_t) + sizeof(byte_with_flags)) {
            /* It is more memory-efficient to start a new swath */
            swaths.emplace_back(remote_address);
        } else {
            /* It is more memory-efficient to write over the intervening space with null values */
            while (remote_delta-- > 1)
                swaths.back().append(0, flags_empty);
        }
        swaths.back().append(remote_memory->u8, flags);
    }

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

    size_t
    size()
    {
        size_t size = 0;
        for(auto& swath : swaths)
            for(auto& f : swath.flags)
                if (f != flags_empty)
                    size += 1;
        return size;
    }

//    size_t
//    size()
//    {
//        size_t size = 0;
//        for(size_t s = 0; s < swaths_count; s++) {
//            for(size_t d = 0; d < swaths[s].data_size; d++)
//                if (swaths[s].data[d].flags > flags_empty)
//                    size += 1;
//        }
//        return size;
//    }

    match_location
    nth_match(size_t n) {
        size_t i = 0;
        for(auto& swath : swaths) {
            /* only actual matches are considered */
            size_t i_inside = 0;
            for(auto& f : swath.flags) {
                if (f > flags_empty) {
                    if(i == n)
                        return match_location{&swaths[i], i_inside};
                    i++;
                }
                i_inside++;
            }
        }

        /* invalid match id */
        return match_location{nullptr, 0};
    }

    //match_location
    //nth_match(size_t n) {
    //    size_t i = 0;
    //    for(size_t s = 0; s < swaths_count; s++) {
    //        /* only actual matches are considered */
    //        size_t i_inside = 0;
    //        for(size_t d = 0; d < swaths[s].data_size; d++) {
    //            if (swaths[s].data[d].flags > flags_empty) {
    //                if (i == n)
    //                    return match_location {&swaths[i], i_inside};
    //                i++;
    //            }
    //            i_inside++;
    //        }
    //    }
    //
    //    /* invalid match id */
    //    return match_location{nullptr, 0};
    //}

//    size_t
//    mem_allo()
//    {
//        size_t size = 0;
//        for(size_t s = 0; s < swaths_count; s++)
//            size += swaths[s].data_allocated;
//        return size+swaths_allocated*(sizeof(swath_t));
//    }
//
//    size_t
//    mem_virt()
//    {
//        size_t size = 0;
//        for(size_t s = 0; s < swaths_count; s++)
//            size += swaths[s].data_size;
//        return size+swaths_count*(sizeof(swath_t));
//    }


    // todo[critical]: must return match_t
    // todo[high]: must return full string, not just 8 bytes
    // todo[med]: must return match with old value
    // handlers.c:375
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
////                    RE::value_t val = m.swath->data_to_val(m.index);
//                    for(size_t i = 0; i < RE::flags_to_memlength(dt, ret.flags); i++) {
//                        /* Both uint8_t, no explicit casting needed */
//                        ret.bytes[i] = m.swath->data[m.index + i].byte;
//                    }
//            }
//        }
//        return ret;
//    }
    
    
    
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
//        writing_swath_index->data_size = 0;
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
//            if (reading_iterator >= reading_swath.data_size) {
//
//                reading_swath_index = (swath_t *)
//                        (&reading_swath_index->data[reading_swath.data_size]);
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
    

//    std::vector<Cregion>  regions;
//    std::string           path;
//    bio::mapped_file      snapshot_mf;
//    bio::mapped_file      flags_mf;
//
//
//    explicit matches_t(const bio::mapped_file& snapshot_mapped_file,
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
    
    bool scan(RE::matches_t& writing_matches,
              const RE::Edata_type& data_type,
              const RE::Cuservalue *uservalue,
              const RE::Ematch_type& match_type);

    bool scan_update(RE::matches_t& writing_matches);

    bool rescan(RE::matches_t& writing_matches,
                const RE::Edata_type& data_type,
                const RE::Cuservalue *uservalue,
                RE::Ematch_type match_type);
    
    bool scan_reset();
};

} //namespace RE
