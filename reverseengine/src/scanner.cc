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
    uintptr_t reg_pos;

    mem64_t *memory_ptr = nullptr;
    size_t match_length;
    RE::match_flags checkflags;

    size_t copied;
    size_t required_extra_bytes_to_record = 0;

    /* check every memory region */
    for(const RE::Cregion& region : handle->regions) {
        /* For every offset, check if we have a match. */
        size_t memlength = region.size;
        size_t buffer_size = 0;
        reg_pos = region.address;

        for ( ; ; memlength--, buffer_size--, reg_pos++, buf_pos++) {
            /* check if the buffer is finished (or we just started) */
            if UNLIKELY(buffer_size == 0) {
                /* the whole region is finished */
                if (memlength == 0) break;

                /* stop scanning if asked to */
                if (stop_flag) break;

                /* load the next buffer block */
                size_t alloc_size = MIN(memlength, MAX_ALLOC_SIZE);
                copied = handle->read(reg_pos, buffer, alloc_size);
                if UNLIKELY(copied < 0)
                    break;
                else if UNLIKELY(copied < alloc_size) {
                    /* the region ends here, update `memlength` */
                    memlength = copied;
                    bzero(buffer + copied, MAX_ALLOC_SIZE - copied);
                }
                /* If less than `MAX_ALLOC_SIZE` bytes remain, we have all of them
                 * in the buffer, so go all the way.
                 * Otherwise we need to stop at `MAX_BUFFER_SIZE`, so that
                 * the last byte we look at has a full VLT after it */
                buffer_size = memlength <= MAX_ALLOC_SIZE ? memlength : MAX_BUFFER_SIZE;
                buf_pos = buffer;
            }

            memory_ptr = reinterpret_cast<mem64_t *>(buf_pos);
            checkflags = flags_empty;

            /* check if we have a match */
            match_length = (*sm_scan_routine)(memory_ptr, memlength, nullptr, uservalue, checkflags);
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



bool
RE::Scanner::scan_next(matches_t& reading_matches,
                       matches_t& writing_matches,
                       const RE::Edata_type& data_type,
                       const RE::Cuservalue *uservalue,
                       RE::Ematch_type match_type)
{
    using namespace std;

    if (!RE::sm_choose_scanroutine(data_type, match_type, uservalue, false)) {
        printf("unsupported scan for current data type.\n");
        return false;
    }

    size_t required_extra_bytes_to_record = 0;
    scan_progress = 0.0;
    stop_flag = false;
    writing_matches.matches_size = 0;

    mem64_t memory_ptr{};

    size_t swaths_remain = reading_matches.swaths_count;
    writing_matches.swaths_count = 0;
    for(size_t s = 0; s < swaths_remain; s++) {
        writing_matches.swaths_count++;
        swath_t& swath = reading_matches.swaths[s];
        size_t bytes_remain = swath.data_size;
        swath.data_size = 0;
        size_t reading_iterator = 0;
        while(bytes_remain) {
            unsigned int match_length = 0;
            RE::match_flags checkflags = flags_empty;

            uint16_t flags = swath.data[reading_iterator].flags;
            size_t old_length;
#if RE_ADJUST_FLAGS_TO_MEMLENGTH_INLINE != 2
            old_length = flags_to_memlength(data_type, flags);
#else
            switch (data_type) {
                case Edata_type::BYTEARRAY:
                case Edata_type::STRING:
                    old_length = flags;
                    [[fallthrough]];
                default: /* NUMBER */
                    old_length = (flags & flags_64b) ? 8 :
                           (flags & flags_32b) ? 4 :
                           (flags & flags_16b) ? 2 :
                           (flags & flags_8b ) ? 1 : 0;
            }
#endif
            uintptr_t address;
#if RE_ADJUST_REMOTE_GET_INLINE != 2
            address = swath.remote_get(reading_iterator);
#else
            address = swath.base_address + static_cast<uintptr_t>(reading_iterator);
#endif

            /* read value from this address */
            size_t memlength = handle->read_cached(address, &memory_ptr, old_length);
            if UNLIKELY(memlength == RE::Handle::npos) {
                /* If we can't look at the data here, just abort the whole recording, something bad happened */
                required_extra_bytes_to_record = 0;
            }
            /* Test only valid old matches */
            else if (flags != flags_empty)  {
                value_t val;
#if RE_ADJUST_DATA_TO_VAL_INLINE != 2
                val = swath.data_to_val_aux(reading_iterator, swath.data_size);
#else
                const auto swath_length = swath.data_size;
                const auto index = reading_iterator;
                const auto& data = swath.data;

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
# if RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL == -1
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
                        [[fallthrough]];
                    case 7:
                        val.bytes[6] = data[index + 6].byte;
                        [[fallthrough]];
                    case 6:
                        val.bytes[5] = data[index + 5].byte;
                        [[fallthrough]];
                    case 5:
                        val.bytes[4] = data[index + 4].byte;
                        [[fallthrough]];
                    case 4:
                        val.bytes[3] = data[index + 3].byte;
                        [[fallthrough]];
                    case 3:
                        val.bytes[2] = data[index + 2].byte;
                        [[fallthrough]];
                    case 2:
                        val.bytes[1] = data[index + 1].byte;
                        [[fallthrough]];
                    case 1:
                        val.bytes[0] = data[index + 0].byte;
                        [[fallthrough]];
                    default:;
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
#endif

                memlength = old_length < memlength ? old_length : memlength;

                checkflags = flags_empty;

                match_length = (*sm_scan_routine)(&memory_ptr, memlength, &val, uservalue, checkflags);
            }

            if (match_length > 0) {
                /* Still a candidate. Write data.
                   - We can get away with overwriting in the same array because it is guaranteed to take up the same number of bytes or fewer,
                     and because we copied out the reading swath metadata already.
                   - We can get away with assuming that the pointers will stay valid,
                     because as we never add more data to the array than there was before, it will not reallocate. */
                writing_matches.add_element(0, &memory_ptr, checkflags);
                writing_matches.matches_size++;
                required_extra_bytes_to_record = match_length - 1;
            }
            else if (required_extra_bytes_to_record) {
                writing_matches.add_element(0, &memory_ptr, flags_empty);
                required_extra_bytes_to_record--;
            }

            bytes_remain--;
            reading_iterator++;
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
