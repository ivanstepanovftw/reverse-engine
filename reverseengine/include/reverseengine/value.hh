/*
    This file is part of Reverse Engine.

    Structs for target process.

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>
    Copyright (C) 2017      Andrea Stacchiotti <andreastacchiotti@gmail.com>

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

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cerrno>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <cstring>
#include <sys/param.h>
#include <bitset>
#include <cstddef>
#include <reverseengine/common.hh>
#include <bitmask/bitmask.hpp>


NAMESPACE_BEGIN(RE)

/** @arg what: any number
 * @return: string number represented as hex */
template <typename T, size_t value_size = sizeof(T), std::endian endianess = std::endian::native>
std::string HEX(const T& value)
{
    using namespace std;
    auto *buffer = (uint8_t *)(&value);
    char converted[value_size * 2 + 1];
    if (endianess == std::endian::big)
        for(size_t i = 0; i < value_size; ++i) {
            sprintf(&converted[i*2], "%02X", buffer[i]);
        }
    else
        for(size_t i = 0; i < value_size; ++i) {
            sprintf(&converted[i*2], "%02X", buffer[value_size-1-i]);
        }
    return converted;
}




enum class region_mode_t : uint8_t
{
    none = 0,
    executable = 1u<<0u,
    writable   = 1u<<1u,
    readable   = 1u<<2u,
    shared     = 1u<<3u,
};
BITMASK_DEFINE_MAX_ELEMENT(region_mode_t, shared)

class Cregion {
public:
    uintptr_t address;
    uintptr_t size{};

    //todo[critical]: flags or region_mode? Tell the difference!!!
    bitmask::bitmask<region_mode_t> flags;  // -> RE::Eregion_mode

    /// File data
    uintptr_t offset{};
    char deviceMajor{}; //fixme char?
    char deviceMinor{}; //fixme char?
    unsigned long inodeFileNumber{}; //fixme unsigned long?
    std::string pathname; //fixme pathname?
    std::string filename;

    bitmask::bitmask<region_mode_t> region_mode;

    Cregion() : address(0), size(0), flags(0), offset(0), deviceMajor(0), deviceMinor(0), inodeFileNumber(0),
    region_mode(region_mode_t::shared|region_mode_t::readable|region_mode_t::writable|region_mode_t::executable) { }


    bool is_good() {
        return address != 0;
    }

    void serialize()
    {
        // TODO 334[medium]: size dependent. Solution: make serialization.
    }

/*
    friend std::ostream& operator<<(std::ostream& outputStream, const Cregion& region)
    {
        return outputStream<<"{"
                           <<"address: "<<HEX(region.address)
                           <<", end: "<<HEX(region.address+region.size)
                           <<", flags: "
                           <<(region.flags&shared?"s":"-")
                           <<(region.flags&readable?"r":"-")
                           <<(region.flags&writable?"w":"-")
                           <<(region.flags&executable?"x":"-")
                           <<", size: "<<HEX(region.size)
                           <<", filename: "<<region.filename
                           <<"}";
    }
*/
};

enum class Edata_type {
    ANYNUMBER,              /* ANYINTEGER or ANYFLOAT */
    ANYINTEGER,             /* INTEGER of whatever width */
    ANYFLOAT,               /* FLOAT of whatever width */
    INTEGER8,
    INTEGER16,
    INTEGER32,
    INTEGER64,
    FLOAT32,
    FLOAT64,
    BYTEARRAY,
    STRING
};

BITMASK_DEFINE_MAX_ELEMENT(Edata_type, STRING)

enum class Ematch_type
{
    MATCHANY,                /* for snapshot */
    /* following: compare with a given value */
    MATCHEQUALTO,
    MATCHNOTEQUALTO,
    MATCHGREATERTHAN,
    MATCHLESSTHAN,
    MATCHRANGE,
    MATCHEXCLUDE,
    /* following: compare with the old value */
    MATCHUPDATE,
    MATCHNOTCHANGED,
    MATCHCHANGED,
    MATCHINCREASED,
    MATCHDECREASED,
    /* following: compare with both given value and old value */
    MATCHINCREASEDBY,
    MATCHDECREASEDBY
};


/* some routines for working with value_t structures */

/* match_flags: they MUST be implemented as an `uint16_t`, the `__packed__` ensures so.
 * They are reinterpreted as a normal integer when scanning for VLT, which is
 * valid for both endians, as the flags are ordered from smaller to bigger.
 * NAMING: Primitive, single-bit flags are called `flag_*`, while aggregates,
 * defined for convenience, are called `flags_*`*/
/* Problem: c++ from enum class to its underlying type
 * Usage: !!(...) <=> (...)
 * #fixcpp
 */
enum class flag_t : uint16_t {
    flags_empty = 0,

    flag_u8  = 1u<<0u,  /* could be an unsigned  8-bit variable (e.g. unsigned char)      */
    flag_i8  = 1u<<1u,  /* could be a    signed  8-bit variable (e.g. signed char)        */
    flag_u16 = 1u<<2u,  /* could be an unsigned 16-bit variable (e.g. unsigned short)     */
    flag_i16 = 1u<<3u,  /* could be a    signed 16-bit variable (e.g. short)              */
    flag_u32 = 1u<<4u,  /* could be an unsigned 32-bit variable (e.g. unsigned int)       */
    flag_i32 = 1u<<5u,  /* could be a    signed 32-bit variable (e.g. int)                */
    flag_u64 = 1u<<6u,  /* could be an unsigned 64-bit variable (e.g. unsigned long long) */
    flag_i64 = 1u<<7u,  /* could be a    signed 64-bit variable (e.g. long long)          */

    flag_f32 = 1u<<8u,  /* could be a 32-bit floating point variable (i.e. float)         */
    flag_f64 = 1u<<9u,  /* could be a 64-bit floating point variable (i.e. double)        */

    flags_i8b  = flag_u8  | flag_i8,
    flags_i16b = flag_u16 | flag_i16,
    flags_i32b = flag_u32 | flag_i32,
    flags_i64b = flag_u64 | flag_i64,

    flags_integer = flags_i8b | flags_i16b | flags_i32b | flags_i64b,
    flags_float   = flag_f32 | flag_f64,
    flags_all     = flags_integer | flags_float,

    flags_8b   = flags_i8b,
    flags_16b  = flags_i16b,
    flags_32b  = flags_i32b | flag_f32,
    flags_64b  = flags_i64b | flag_f64,

    flags_max = 0xffffu,
    _bitmask_value_mask = flags_max
};
BITMASK_DEFINE(flag_t);

class flag : public bitmask::bitmask<flag_t> {
public:
    using bitmask::bitmask;

    // oliora/bitmask#4, message#4, issue n.1
    constexpr flag(const bitmask<value_type>& flag) noexcept : bitmask::bitmask<value_type>(flag) {}

    /* Possible flags per scan data type: if an incoming uservalue has none of the
     * listed flags we're sure it's not going to be matched by the scan,
     * so we reject it without even trying */
    constexpr static flag convert(const Edata_type& dt) {
        switch (dt) {
            case Edata_type::ANYNUMBER:  return flag_t::flags_all;
            case Edata_type::ANYINTEGER: return flag_t::flags_integer;
            case Edata_type::ANYFLOAT:   return flag_t::flags_float;
            case Edata_type::INTEGER8:   return flag_t::flags_i8b;
            case Edata_type::INTEGER16:  return flag_t::flags_i16b;
            case Edata_type::INTEGER32:  return flag_t::flags_i32b;
            case Edata_type::INTEGER64:  return flag_t::flags_i64b;
            case Edata_type::FLOAT32:    return flag_t::flag_f32;
            case Edata_type::FLOAT64:    return flag_t::flag_f64;
            case Edata_type::BYTEARRAY:  return flag_t::flags_max;
            case Edata_type::STRING:     return flag_t::flags_max;
            default: return flag_t::flags_empty;
        }
    }

    constexpr size_t memlength(const Edata_type& scan_data_type) const
    {
        switch (scan_data_type) {
            case Edata_type::BYTEARRAY:
            case Edata_type::STRING:
                return this->bits();
            default: /* NUMBER */
                return (*this & flag_t::flags_64b) ? 8 :
                       (*this & flag_t::flags_32b) ? 4 :
                       (*this & flag_t::flags_16b) ? 2 :
                       (*this & flag_t::flags_8b ) ? 1 : 0;
        }
    }

    //constexpr flag(const Edata_type& dt) noexcept {
    //    switch (dt) {
    //        case Edata_type::EXECUTE_BIT: flag(flag_t::execute);
    //        case Edata_type::WRITE_BIT:   flag(flag_t::write);
    //        case Edata_type::READ_BIT:    flag(flag_t::read);
    //        default: flag(flag_t::none);
    //    }
    //}
    //
    std::string str() const {
        std::ostringstream ss;
        ss<<(*this & flag_t::flag_u8 ? "C" : "");
        ss<<(*this & flag_t::flag_i8 ? "c" : "");
        ss<<(*this & flag_t::flag_u16 ? "S" : "");
        ss<<(*this & flag_t::flag_i16 ? "s" : "");
        ss<<(*this & flag_t::flag_u32 ? "I" : "");
        ss<<(*this & flag_t::flag_i32 ? "i" : "");
        ss<<(*this & flag_t::flag_u64 ? "L" : "");
        ss<<(*this & flag_t::flag_i64 ? "l" : "");
        ss<<(*this & flag_t::flag_f32 ? "f" : "");
        ss<<(*this & flag_t::flag_f64 ? "d" : "");
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const flag& flag) {
        return os<<flag.str();
    }
};



/* This union describes 8 bytes retrieved from target memory.
 * Pointers to this union are the only ones that are allowed to be unaligned:
 * to avoid performance degradation/crashes on arches that don't support unaligned access
 * (e.g. ARM) we access unaligned memory only through the attributes of this packed union.
 * As described in http://www.alfonsobeato.net/arm/how-to-access-safely-unaligned-data/ ,
 * a packed structure forces the compiler to write general access methods to its members
 * that don't depend on alignment.
 * So NEVER EVER dereference a mem64_t*, but use its accessors to obtain the needed type.
 */
union mem64_t
{
    int8_t   i8;
    uint8_t  u8;
    int16_t  i16;
    uint16_t u16;
    int32_t  i32;
    uint32_t u32;
    int64_t  i64;
    uint64_t u64;
    float    f32;
    double   f64;
    uint8_t bytes[sizeof(int64_t)];
    char    chars[sizeof(int64_t)];
};

/* Matches a memory area given by `memory_ptr` and `memlength` against `user_value` or `old_value`
 * (or both, depending on the matching type), stores the result into saveflags.
 * NOTE: saveflags must be set to 0, since only useful bits are set, but extra bits are not cleared!
 * Returns the number of bytes needed to store said match, 0 for not matched
 */

/* this struct describes matched values */
class value_t {
public:
    uintptr_t address;

    union {
        int8_t   i8;
        uint8_t  u8;
        int16_t  i16;
        uint16_t u16;
        int32_t  i32;
        uint32_t u32;
        int64_t  i64;
        uint64_t u64;
        float    f32;
        double   f64;
        uint8_t bytes[sizeof(int64_t)];
        char    chars[sizeof(int64_t)];
    };

    flag flags;

    explicit value_t() : address(0), u64(0), flags(flag_t::flags_empty) { }

    [[gnu::always_inline]]
    flag nearest_flag() const
    {
        if (flags & flag_t::flag_i64) return flag_t::flag_i64;
        if (flags & flag_t::flag_i32) return flag_t::flag_i32;
        if (flags & flag_t::flag_i16) return flag_t::flag_i16;
        if (flags & flag_t::flag_i8)  return flag_t::flag_i8;
        if (flags & flag_t::flag_u64) return flag_t::flag_u64;
        if (flags & flag_t::flag_u32) return flag_t::flag_u32;
        if (flags & flag_t::flag_u16) return flag_t::flag_u16;
        if (flags & flag_t::flag_u8)  return flag_t::flag_u8;
        if (flags & flag_t::flag_f64) return flag_t::flag_f64;
        if (flags & flag_t::flag_f32) return flag_t::flag_f32;
        return flag_t::flags_empty;
    }

    [[gnu::always_inline]]
    std::string flag2str() const
    {
        if (flags & flag_t::flag_i64) return "i64";
        if (flags & flag_t::flag_i32) return "i32";
        if (flags & flag_t::flag_i16) return "i16";
        if (flags & flag_t::flag_i8)  return "i8";
        if (flags & flag_t::flag_u64) return "u64";
        if (flags & flag_t::flag_u32) return "u32";
        if (flags & flag_t::flag_u16) return "u16";
        if (flags & flag_t::flag_u8)  return "u8";
        if (flags & flag_t::flag_f64) return "f64";
        if (flags & flag_t::flag_f32) return "f32";
        return "";
    }

    [[gnu::always_inline]]
    std::string val2str() const
    {
        const mem64_t *mem = reinterpret_cast<const mem64_t *>(this->bytes);
        if (flags & flag_t::flag_i64) return std::to_string(mem->i64);
        if (flags & flag_t::flag_i32) return std::to_string(mem->i32);
        if (flags & flag_t::flag_i16) return std::to_string(mem->i16);
        if (flags & flag_t::flag_i8)  return std::to_string(mem->i8);
        if (flags & flag_t::flag_u64) return std::to_string(mem->u64);
        if (flags & flag_t::flag_u32) return std::to_string(mem->u32);
        if (flags & flag_t::flag_u16) return std::to_string(mem->u16);
        if (flags & flag_t::flag_u8)  return std::to_string(mem->u8);
        if (flags & flag_t::flag_f64) return std::to_string(mem->f64);
        if (flags & flag_t::flag_f32) return std::to_string(mem->f32);
        if (flags.bits() & 0) return "0";
        return "";
    }

    [[gnu::always_inline]]
    std::string address2str() const
    {
        if (address <= 0xFFFF'FFFF)
            return RE::HEX<decltype(address), 4>(address);
        else if (address <= 0xFFFF'FFFF'FFFF)
            return RE::HEX<decltype(address), 6>(address);
        else
            return RE::HEX<>(address);
    }

    std::string str() const
    {
        std::ostringstream ss;
        ss<<address2str()<<": "<<val2str()<<", "<<flag2str();
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& out, const value_t& t)
    {
        return out<<t.str();
    }
};


/* bytearray wildcards: they must be uint8_t. They are ANDed with the incoming
 * memory before the comparison, so that '??' wildcards always return true
 * It's possible to extend them to fully granular wildcard-ing, if needed */
enum wildcard_t
{
    FIXED = 0xffu,
    WILDCARD = 0x00u,
};


/* this struct describes values provided by users */
class Cuservalue
{
public:
    int8_t   i8;
    uint8_t  u8;
    int16_t  i16;
    uint16_t u16;
    int32_t  i32;
    uint32_t u32;
    int64_t  i64;
    uint64_t u64;
    float    f32;
    double   f64;
    
    uint8_t *bytearray_value;
    wildcard_t *wildcard_value;
//    std::vector<uint8_t> bytearray_value;
//    std::vector<wildcard_t> wildcard_value;
    
    char *string_value;
//    std::string string_value;
//    std::wstring wstring_value;
    
    flag flags;

    size_t parse_uservalue_int(const std::string& text);

    size_t parse_uservalue_float(const std::string& text);

/* parse int or float */
    size_t parse_uservalue_number(const std::string& text);

    size_t parse_uservalue_bytearray(const std::string& text);

    size_t parse_uservalue_string(const std::string& text);
};

NAMESPACE_END(RE)
