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

NAMESPACE_BEGIN(RE)

typedef unsigned int (*scan_routine_t)(const RE::mem64_t *memory_ptr,
                                       size_t memlength,
                                       const RE::value_t *old_value,
                                       const RE::Cuservalue *user_value,
                                       RE::flag& saveflags);
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
                                  const flag& uflags,
                                  bool reverse_endianness);

NAMESPACE_END(RE)
