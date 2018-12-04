/*
    This file is part of Reverse Engine.

    

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>
    Copyright (C) 2015,2017 Sebastian Parschauer <s.parschauer@gmx.de>
    Copyright (C) 2010      WANG Lu  <coolwanglu(a)gmail.com>
    Copyright (C) 2009      Eli Dupree  <elidupree(a)charter.net>

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

#include <reverseengine/scanner.hh>
#include <reverseengine/value.hh>
#include <reverseengine/scanroutines.hh>
#include <reverseengine/common.hh>


void
RE::Scanner::string_to_uservalue(const RE::Edata_type &data_type,
                                 const std::string &text,
                                 RE::Ematch_type *match_type,
                                 RE::Cuservalue *uservalue)
{
    using namespace std;
    
    if (text.empty())
        throw bad_uservalue_cast(text,0);
    
    switch (data_type) {
        case RE::Edata_type::BYTEARRAY:
//            if (!parse_uservalue_bytearray(text, &uservalue[0])) {
//                return false;
//            }
            return;
        case RE::Edata_type::STRING:
            uservalue[0].string_value = const_cast<char *>(text.c_str());
            uservalue[0].flags = flags_max;
            return;
        default:
            // ╔════╤════════════╤═══════╤═══════════════════════════╤═════════════════════════════════════╗
            // ║    │ C operator │ Alias │ Description               │ Description (if no number provided) ║
            // ╠════╪════════════╪═══════╪═══════════════════════════╪═════════════════════════════════════╣
            // ║ a1 │            │ ?     │ unknown initial value     │                                     ║
            // ║ a2 │ ++         │       │ increased                 │                                     ║
            // ║ a3 │ --         │       │ decreased                 │                                     ║
            // ╟────┼────────────┼───────┼───────────────────────────┼─────────────────────────────────────╢
            // ║ b1 │ == N       │ N     │ exactly N                 │ not changed                         ║
            // ║ b2 │ != N       │       │ not N                     │ changed                             ║
            // ║ b3 │ += N       │       │ increased by N            │ increased                           ║
            // ║ b4 │ -= N       │       │ decreased by N            │ decreased                           ║
            // ║ b5 │ > N        │       │ greater than N            │ increased (just for ux)             ║
            // ║ b6 │ < N        │       │ less than N               │ decreased (just for ux)             ║
            // ╟────┼────────────┼───────┼───────────────────────────┼─────────────────────────────────────╢
            // ║ d  │            │ N..M  │ range                     │                                     ║
            // ╚════╧════════════╧═══════╧═══════════════════════════╧═════════════════════════════════════╝
            string pattern(text.size(), '\0');
            
            //remove all spaces
            auto it = copy_if(text.begin(),
                              text.end(),
                              pattern.begin(),
                              [](int i) { return (!isspace(i) && i != '_' && i != '\''); });
            pattern.resize(static_cast<size_t>(distance(pattern.begin(), it)));
            
            auto boilerplate_a = [&](const string &MASK, RE::Ematch_type MATCH_TYPE_NO_VALUE) -> bool
            {
                size_t cursor = pattern.find(MASK);
                if (cursor == string::npos)  // if mask not found
                    return false;
                cursor += MASK.size();
                if (cursor != pattern.size())  // if extra symbols after
                    throw bad_uservalue_cast(pattern, cursor);
                *match_type = MATCH_TYPE_NO_VALUE;
                return true;
            };
            if (boilerplate_a("?", RE::Ematch_type::MATCHANY))        goto valid_number;  // a1
            if (boilerplate_a("++", RE::Ematch_type::MATCHINCREASED)) goto valid_number;  // a2
            if (boilerplate_a("--", RE::Ematch_type::MATCHDECREASED)) goto valid_number;  // a3
            
            auto boilerplate_b = [&](const string &MASK, RE::Ematch_type MATCH_TYPE, RE::Ematch_type MATCH_TYPE_NO_VALUE) -> bool
            {
                size_t cursor = pattern.find(MASK);
                if (cursor == string::npos)  // if mask not found
                    return false;
                cursor += MASK.size();
                if (cursor == pattern.size()) {  // end reached
                    *match_type = MATCH_TYPE_NO_VALUE;
                    return true;
                }
                const string &possible_number = pattern.substr(cursor);  // slice [cursor:-1]
                size_t pos = parse_uservalue_number(possible_number, &uservalue[0]);
                if (pos != possible_number.size())  // if parse_uservalue_number() not parsed whole possible_number
                    throw bad_uservalue_cast(pattern, cursor + pos);
                *match_type = MATCH_TYPE;
                return true;
            };
            if (boilerplate_b("==", RE::Ematch_type::MATCHEQUALTO,     RE::Ematch_type::MATCHNOTCHANGED)) goto valid_number; // 1
            if (boilerplate_b("!=", RE::Ematch_type::MATCHNOTEQUALTO,  RE::Ematch_type::MATCHCHANGED))    goto valid_number; // 2
            if (boilerplate_b("+=", RE::Ematch_type::MATCHINCREASEDBY, RE::Ematch_type::MATCHINCREASED))  goto valid_number; // 3
            if (boilerplate_b("-=", RE::Ematch_type::MATCHDECREASEDBY, RE::Ematch_type::MATCHDECREASED))  goto valid_number; // 4
            if (boilerplate_b(">",  RE::Ematch_type::MATCHGREATERTHAN, RE::Ematch_type::MATCHINCREASED))  goto valid_number; // 5
            if (boilerplate_b("<",  RE::Ematch_type::MATCHLESSTHAN,    RE::Ematch_type::MATCHDECREASED))  goto valid_number; // 6
            
            
            // todo MATCHNOTINRANGE "!= 0..1", MATCHNOTINRANGE / MATCHEXCLUDE
            auto boilerplate_c = [&](const string &MASK, RE::Ematch_type MATCH_TYPE) -> bool
            {
                size_t cursor = pattern.find(MASK);
                if (cursor == string::npos)   // if mask not found
                    return false;
                
                /// Parse first number N..M
                ///                    ^
                const string &possible_number = pattern.substr(0, cursor-0);  // slice [0:cursor]
                clog<<"pattern.substr(0, cursor): \""<<pattern.substr(0, cursor)<<"\""<<endl;
                clog<<"pattern.substr(0, cursor+1): \""<<pattern.substr(0, cursor+1)<<"\""<<endl;
                size_t pos = parse_uservalue_number(possible_number, &uservalue[0]);
                if (pos != possible_number.size())  // if parse_uservalue_number() not parsed whole possible_number
                    throw bad_uservalue_cast(pattern, 0+pos);
                cursor += MASK.size();
                
                /// Parse last number N..M
                ///                      ^
                const string &possible_number2 = pattern.substr(cursor);  // slice [cursor:-1]
                size_t pos2 = parse_uservalue_number(possible_number2, &uservalue[1]);
                if (pos2 != possible_number2.size())
                    throw bad_uservalue_cast(pattern, cursor+pos2);
                
                /// Check that the range is nonempty
                if (uservalue[0].f64 > uservalue[1].f64) {
                    clog<<"Empty range"<<endl;
                    throw bad_uservalue_cast(pattern, cursor);
                }
                /// Store the bitwise AND of both flags in the first value
                uservalue[0].flags &= uservalue[1].flags;
                *match_type = MATCH_TYPE;
                return true;
            };
            if (boilerplate_c("..",  RE::Ematch_type::MATCHRANGE)) goto valid_number;
            
            // ==
            {
                size_t pos = parse_uservalue_number(pattern, &uservalue[0]);
                if (pos != pattern.size()) { // if parse_uservalue_number() not parsed whole possible_number
                    throw bad_uservalue_cast(pattern, pos);
                }
                *match_type = RE::Ematch_type::MATCHEQUALTO;
            }
    }

valid_number:;
//fixme[high]: data_type_to_flags must be a function
    uservalue[0].flags &= RE::data_type_to_flags[(uint16_t)data_type];
    uservalue[1].flags &= RE::data_type_to_flags[(uint16_t)data_type];
    return;
}



bio::mapped_file
RE::Scanner::make_snapshot(const std::string& path)
{
    using namespace std;
    
    /// Allocate space
    if (handle->regions.empty())
        throw invalid_argument("no regions defined");
    
    uintptr_t total_scan_bytes = 0;
    for(const RE::Cregion& region : handle->regions)
        total_scan_bytes += region.size;
    if (total_scan_bytes == 0)
        throw invalid_argument("zero bytes to scan");
    
    /// Create mmap
    bio::mapped_file_params snapshot_mf_(path);
    snapshot_mf_.flags         = bio::mapped_file::mapmode::readwrite;
    snapshot_mf_.length        = total_scan_bytes + handle->regions.size()*2*sizeof(uintptr_t);
    snapshot_mf_.new_file_size = total_scan_bytes + handle->regions.size()*2*sizeof(uintptr_t);
    
    bio::mapped_file snapshot_mf(snapshot_mf_);
    if (!snapshot_mf.is_open())
        throw invalid_argument("cant open '"+path+"'");
    
    char *snapshot_begin = snapshot_mf.data();
    char *snapshot = snapshot_begin;
    ssize_t copied;
    
    /// Snapshot goes here
    for(RE::Cregion region : handle->regions) {
        copied = handle->read(region.address, snapshot+2*sizeof(uintptr_t), region.size);
        if (copied < 0) {
//            clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
//            if (!handle->is_running())
//                throw invalid_argument("process not running");
            total_scan_bytes -= region.size;
            continue;
        } else if (copied != region.size) {
//            clog<<"warning: region: "<<region<<", requested: "<<HEX(region.size)<<", copied "<<HEX(copied)<<endl;
            region.size = static_cast<uintptr_t>(copied);
        }
        memcpy(snapshot,
               &region.address,
               2*sizeof(region.address));
        snapshot += 2*sizeof(region.address) + region.size;
    }
    snapshot_mf.resize(snapshot - snapshot_mf.data());
    
    return snapshot_mf;
}


bool
RE::Scanner::scan(matches_t& writing_matches,
                  const Edata_type& data_type,
                  const Cuservalue *uservalue,
                  const Ematch_type& match_type)
{
    using namespace std;
    using namespace std::chrono;

//    high_resolution_clock::time_point timestamp;
//    double i_total = 0;

    if (!RE::sm_choose_scanroutine(data_type, match_type, uservalue, false)) {
        printf("unsupported scan for current data type.\n");
        return false;
    }

    scan_progress = 0.0;
    stop_flag = false;

    /* The maximum logical size is a comfortable 1MiB (increasing it does not help).
     * The actual allocation is that plus the rounded size of the maximum possible VLT.
     * This is needed because the last byte might be scanned as max size VLT,
     * thus need 2^16 - 2 extra bytes after it */
    constexpr size_t MAX_BUFFER_SIZE = 128*KiB;
    constexpr size_t MAX_ALLOC_SIZE = MAX_BUFFER_SIZE+64*KiB;

    /* allocate data array */
    uint8_t *buffer = new uint8_t[MAX_ALLOC_SIZE];
    uint8_t *buf_pos = buffer;

    size_t required_extra_bytes_to_record = 0;

    /* check every memory region */
    for(const RE::Cregion& region : handle->regions) {
        /* For every offset, check if we have a match. */
        size_t memlength = region.size;
        size_t buffer_size = 0;
        uintptr_t reg_pos = region.address;

        for ( ; ; memlength--, buffer_size--, reg_pos++, buf_pos++) {
            /* check if the buffer is finished (or we just started) */
            if UNLIKELY(buffer_size == 0) {
                /* the whole region is finished */
                if (memlength == 0) break;

                /* stop scanning if asked to */
                if (stop_flag) break;

                /* load the next buffer block */
                size_t alloc_size = MIN(memlength, MAX_ALLOC_SIZE);
                size_t copied = handle->read(reg_pos, buffer, alloc_size);
                if UNLIKELY(copied == Handle::npos)
                    break;
                else if UNLIKELY(copied < alloc_size) {
                    /* the region ends here, update `memlength` */
                    memlength = copied;
                    memset(buffer + copied, 0, MAX_ALLOC_SIZE - copied);
                }
                /* If less than `MAX_ALLOC_SIZE` bytes remain, we have all of them
                 * in the buffer, so go all the way.
                 * Otherwise we need to stop at `MAX_BUFFER_SIZE`, so that
                 * the last byte we look at has a full VLT after it */
                buffer_size = memlength <= MAX_ALLOC_SIZE ? memlength : MAX_BUFFER_SIZE;
                buf_pos = buffer;
            }

            mem64_t *memory_ptr = reinterpret_cast<mem64_t *>(buf_pos);
            RE::match_flags checkflags = flags_empty;

            /* check if we have a match */
            size_t match_length = (*sm_scan_routine)(memory_ptr, memlength, nullptr, uservalue, checkflags);
            if UNLIKELY(match_length > 0) {
                writing_matches.add_element(reg_pos, memory_ptr, checkflags);
                writing_matches.matches_size++;
                required_extra_bytes_to_record = match_length - 1;
            }
            else if UNLIKELY(required_extra_bytes_to_record > 0) {
                writing_matches.add_element(reg_pos, memory_ptr, flags_empty);
                required_extra_bytes_to_record--;
            }
        }
    }
    delete[] buffer;
    scan_progress = 1.0;
    return true;
}


bool RE::Scanner::scan_update(RE::matches_t& writing_matches) {
    // Invalidate cache to get fresh values
    handle->base = 0;

    for (swath_t& s : writing_matches.swaths) {
        size_t copied = handle->read_cached(s.base_address, &s.bytes[0], s.bytes.size());
        /* check if the read succeeded */
        if UNLIKELY(copied == Handle::npos) {
            //cout<<"Resizing swath "<<HEX(s.base_address)<<" from "<<s.bytes.size()<<" to "<<0<<" elements"<<endl;
            //cout<<"Error: can not read "<<s.bytes.size()<<" bytes from "<<HEX(s.base_address)<<": "<<strerror(errno)<<endl;
            s.bytes.resize(0);
            s.flags.resize(0);
        } else if UNLIKELY(copied < s.bytes.size()) {
            /* go ahead with the partial read and stop the gathering process */
            s.bytes.resize(copied);
            s.flags.resize(copied);
        }
        const size_t len = s.flags.size();
        if (len > 6) s.flags[len-7] &= ~flags_64b;
        if (len > 5) s.flags[len-6] &= ~flags_64b;
        if (len > 4) s.flags[len-5] &= ~flags_64b;
        if (len > 3) s.flags[len-4] &= ~flags_64b;
        if (len > 2) s.flags[len-3] &= ~flags_32b;
        if (len > 1) s.flags[len-2] &= ~flags_32b;
        if (len > 0) s.flags[len-1] &= ~flags_16b;
    }

    return handle->base != 0;
}


bool
RE::Scanner::rescan(matches_t& writing_matches,
                    const RE::Edata_type& data_type,
                    const RE::Cuservalue *uservalue,
                    RE::Ematch_type match_type)
{
    using namespace std;

    if (!RE::sm_choose_scanroutine(data_type, match_type, uservalue, false)) {
        printf("unsupported scan for current data type.\n");
        return false;
    }

    writing_matches.matches_size = 0;
    scan_progress = 0.0;
    stop_flag = false;

    for (swath_t& s : writing_matches.swaths) {
        for (size_t it = 0; it < s.bytes.size(); it++) {
            mem64_t *mem = reinterpret_cast<mem64_t *>(&s.bytes[it]);
            uint16_t flag = s.flags[it];
            size_t mem_size = flags_to_memlength(data_type, flag);

            RE::match_flags checkflags = flags_empty;
            unsigned int match_length = 0;

            if (flag != flags_empty) {
                /* Test only valid old matches */
                value_t val;
                val = s.data_to_val_aux(it);

                match_length = (*sm_scan_routine)(mem, mem_size, &val, uservalue, checkflags);
                s.flags[it] = checkflags;
            }

            if (match_length > 0) {
                writing_matches.matches_size++;
            } else {
                //cout<<"match_length == 0"<<endl;
            }
        }
    }

    return true;
}



bool
RE::Scanner::scan_reset()
{
//    snapshots.clear();
//    matches.clear();
    return true;
}
