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

#ifndef RE_VALUE_HH
#define RE_VALUE_HH

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


#ifdef __GNUC__
# define LIKELY(x)     __builtin_expect(!!(x), 1)
# define UNLIKELY(x)   __builtin_expect(!!(x), 0)
#else
# define LIKELY(x)     (x)
# define UNLIKELY(x)   (x)
#endif

template <typename T>
std::string HEX(T&& what)
{
    using namespace std;
    stringstream ss;
    ss<<"0x"<<hex<<setw(sizeof(T))<<setfill('0')<<noshowbase<<what<<dec;
//    ss<<hex<<showbase<<what<<dec;
    return ss.str();
}


enum region_mode_t : uint8_t
{
    readable   = 1u<<0u,
    writable   = 1u<<1u,
    executable = 1u<<2u,
    shared     = 1u<<3u,
};

class region_t
{
public:
    uintptr_t address;
    uintptr_t size;
    
    uint8_t flags;
    
    /// File data
    uintptr_t offset;
    char deviceMajor;
    char deviceMinor;
    unsigned long inodeFileNumber;
    std::string pathname;
    std::string filename;
    
    void serialize()
    {
        // TODO 334. Problem [high]: size dependent. Solution: [low] make serialization.
    }
    
    friend std::ostream& operator<<(std::ostream& outputStream, const region_t& region)
    {
        return outputStream<<"{"
                           <<"filename: '"<<region.filename
                           <<"', address: "<<HEX(region.address)
                           <<", size: "<<HEX(region.size)
                           <<", flags: "<<std::bitset<4>(region.flags)
                           <<"}";
    }
};

enum scan_data_type_t
{
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

enum scan_match_type_t
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
//todo fit in 1 byte
enum match_flags : uint16_t
{
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
    
    flags_max = 0xffffu
};


/* Possible flags per scan data type: if an incoming uservalue has none of the
 * listed flags we're sure it's not going to be matched by the scan,
 * so we reject it without even trying */
static uint16_t scan_data_type_to_flags[] = {
        [ANYNUMBER]  = flags_all,
        [ANYINTEGER] = flags_integer,
        [ANYFLOAT]   = flags_float,
        [INTEGER8]   = flags_i8b,
        [INTEGER16]  = flags_i16b,
        [INTEGER32]  = flags_i32b,
        [INTEGER64]  = flags_i64b,
        [FLOAT32]    = flag_f32,
        [FLOAT64]    = flag_f64,
        [BYTEARRAY]  = flags_max,
        [STRING]     = flags_max
};

static inline size_t flags_to_memlength(scan_data_type_t scan_data_type, uint16_t flags)
{
    switch (scan_data_type) {
        case BYTEARRAY:
        case STRING:
            return flags;
        default: /* NUMBER */
            return (flags & flags_64b) ? 8 :
                   (flags & flags_32b) ? 4 :
                   (flags & flags_16b) ? 2 :
                   (flags & flags_8b ) ? 1 : 0;
    }
}

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
    int8_t  chars[sizeof(int64_t)];
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
struct uservalue_t
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
    
    uint8_t *bytearray_value;
    wildcard_t *wildcard_value;
//    std::vector<uint8_t> bytearray_value;
//    std::vector<wildcard_t> wildcard_value;
    
    char *string_value;
//    std::string string_value;
//    std::wstring wstring_value;
    
    uint16_t flags;
};

size_t parse_uservalue_int(const std::string& text, uservalue_t *uservalue);

size_t parse_uservalue_float(const std::string& text, uservalue_t *uservalue);

size_t parse_uservalue_number(const std::string& text, uservalue_t *uservalue);     // parse int or float

size_t parse_uservalue_bytearray(const std::string& text, uservalue_t *uservalue);

size_t parse_uservalue_string(const std::string& text, uservalue_t *uservalue);

#endif //RE_VALUE_HH
