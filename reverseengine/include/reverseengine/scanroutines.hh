/*
    Definition of routines of scanning for different data types.

    Copyright (C) 2009,2010 WANG Lu  <coolwanglu(a)gmail.com>
    Copyright (C) 2015      Vyacheslav Shegai <v.shegai(a)netris.ru>

    This file is part of libscanmem.

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

#include <stdbool.h>

#include "value.hh"

namespace RE {

/* Matches a memory area given by `memory_ptr` and `memlength` against `user_value` or `old_value`
 * (or both, depending on the matching type), stores the result into saveflags.
 * NOTE: saveflags must be set to 0, since only useful bits are set, but extra bits are not cleared!
 * Returns the number of bytes needed to store said match, 0 for not matched
 */

/* this struct describes matched values */
class value_t {
public:
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
        uint8_t  bytes[sizeof(int64_t)];
        char     chars[sizeof(int64_t)];
    };
    
    uint16_t flags;
};

//int main() {
//    value_t *value;
//    value->u8;
//}

typedef unsigned int (*scan_routine_t)(const RE::mem64_t *memory_ptr,
                                       size_t memlength,
                                       const RE::value_t *old_value,
                                       const RE::Cuservalue *user_value,
                                       RE::match_flags& saveflags);
extern scan_routine_t sm_scan_routine;

/* 
 * Choose the global scanroutine according to the given parameters, sm_scan_routine will be set.
 * Returns whether a proper routine has been found.
 */
bool sm_choose_scanroutine(RE::Edata_type dt,
                           RE::Ematch_type mt,
                           const RE::Cuservalue* uval,
                           bool reverse_endianness);

scan_routine_t sm_get_scanroutine(RE::Edata_type dt,
                                  RE::Ematch_type mt,
                                  uint16_t uflags,
                                  bool reverse_endianness);

} //namespace RE
