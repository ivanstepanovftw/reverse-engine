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

#include "scanner.hh"
#include "value.hh"
#include "scanroutines.hh"


namespace bio = boost::iostreams;


void
Scanner::string_to_uservalue(const scan_data_type_t &data_type,
                             const std::string &text,
                             scan_match_type_t *match_type,
                             uservalue_t *uservalue)
{
    using namespace std;
    
    if (text.empty())
        throw bad_uservalue_cast(text,0);
    
    switch (data_type) {
        case BYTEARRAY:
//            if (!parse_uservalue_bytearray(text, &uservalue[0])) {
//                return false;
//            }
            return;
        case STRING:
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
            
            auto boilerplate_a = [&](const string &MASK, scan_match_type_t MATCH_TYPE_NO_VALUE) -> bool
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
            if (boilerplate_a("?", MATCHANY))        goto valid_number;  // a1
            if (boilerplate_a("++", MATCHINCREASED)) goto valid_number;  // a2
            if (boilerplate_a("--", MATCHDECREASED)) goto valid_number;  // a3
            
            auto boilerplate_b = [&](const string &MASK, scan_match_type_t MATCH_TYPE, scan_match_type_t MATCH_TYPE_NO_VALUE) -> bool
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
            if (boilerplate_b("==", MATCHEQUALTO,     MATCHNOTCHANGED)) goto valid_number; // 1
            if (boilerplate_b("!=", MATCHNOTEQUALTO,  MATCHCHANGED))    goto valid_number; // 2
            if (boilerplate_b("+=", MATCHINCREASEDBY, MATCHINCREASED))  goto valid_number; // 3
            if (boilerplate_b("-=", MATCHDECREASEDBY, MATCHDECREASED))  goto valid_number; // 4
            if (boilerplate_b(">",  MATCHGREATERTHAN, MATCHINCREASED))  goto valid_number; // 5
            if (boilerplate_b("<",  MATCHLESSTHAN,    MATCHDECREASED))  goto valid_number; // 6
            
            
            // todo MATCHNOTINRANGE "!= 0..1", MATCHNOTINRANGE / MATCHEXCLUDE
            auto boilerplate_c = [&](const string &MASK, scan_match_type_t MATCH_TYPE) -> bool
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
            if (boilerplate_c("..",  MATCHRANGE)) goto valid_number;
            
            // ==
            {
                size_t pos = parse_uservalue_number(pattern, &uservalue[0]);
                if (pos != pattern.size()) { // if parse_uservalue_number() not parsed whole possible_number
                    throw bad_uservalue_cast(pattern, pos);
                }
                *match_type = MATCHEQUALTO;
            }
    }

valid_number:;
    uservalue[0].flags &= scan_data_type_to_flags[data_type];
    uservalue[1].flags &= scan_data_type_to_flags[data_type];
    return;
}



bio::mapped_file
Scanner::make_snapshot(const std::string& path)
{
    using namespace std;
    
    /// Allocate space
    if (handle->regions.empty())
        throw invalid_argument("no regions defined");
    
    uintptr_t total_scan_bytes = 0;
    for(const region_t& region : handle->regions)
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
    for(region_t region : handle->regions) {
        copied = handle->read(snapshot+2*sizeof(uintptr_t), region.address, region.size);
        if (copied < 0) {
//            clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
//            if (!handle->is_running())
//                throw invalid_argument("process not running");
            total_scan_bytes -= region.size;
            continue;
        } else if (copied != region.size) {
            clog<<"warning: region: "<<region<<", requested: "<<HEX(region.size)<<", copied "<<HEX(copied)<<endl;
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
Scanner::scan(matches_t& matches_sink,
              const scan_data_type_t& data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t& match_type)
{
    using namespace std;
    
    if (!sm_choose_scanroutine(data_type, match_type, uservalue, 0)) {
        printf("unsupported scan for current data type.\n");
        return false;
    }
    
    scan_progress = 0.0;
    stop_flag = false;
    
    /* The maximum logical size is a comfortable 1MiB (increasing it does not help).
     * The actual allocation is that plus the rounded size of the maximum possible VLT.
     * This is needed because the last byte might be scanned as max size VLT,
     * thus need 2^16 - 2 extra bytes after it */
    constexpr size_t MAX_BUFFER_SIZE = 32 * 1024 * 1024;        // 32 MiB = 0x2000000
//    constexpr size_t MAX_BUFFER_SIZE = 1u<<20u;
    constexpr size_t MAX_ALLOC_SIZE = MAX_BUFFER_SIZE + (1u<<16u);
    
    /* allocate data array */
    uint8_t *buffer = new uint8_t[MAX_ALLOC_SIZE];
    uint8_t *buf_pos = buffer;
    uintptr_t reg_pos;
    
    const mem64_t *memory_ptr = nullptr;
    unsigned int match_length;
    uint16_t checkflags;
    
    ssize_t copied;
    size_t required_extra_bytes_to_record = 0;
    
    /* check every memory region */
    for(const region_t& region : handle->regions) {
        /* For every offset, check if we have a match. */
        size_t memlength = region.size;
        size_t buffer_size = 0;
        reg_pos = region.address;
        
        for ( ; ; memlength--, buffer_size--, reg_pos++, buf_pos++) {
            /* check if the buffer is finished (or we just started) */
            if (UNLIKELY(buffer_size == 0)) {
                /* the whole region is finished */
                if (memlength == 0) break;
                
                /* stop scanning if asked to */
                if (stop_flag) break;
                
                /* load the next buffer block */
                size_t alloc_size = MIN(memlength, MAX_ALLOC_SIZE);
                copied = handle->read(buffer, reg_pos, alloc_size);
                if (UNLIKELY(copied < 0))
                    break;
                else if (UNLIKELY(copied < alloc_size)) {
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
            match_length = (*sm_scan_routine)(memory_ptr, memlength, NULL, uservalue, checkflags);
            if (UNLIKELY(match_length > 0)) {
                assert(match_length <= memlength);
                matches_sink.add_element(reg_pos, memory_ptr, checkflags);
                matches_sink.matches_size++;
                required_extra_bytes_to_record = match_length - 1;
            }
            else if (required_extra_bytes_to_record) {
                matches_sink.add_element(reg_pos, memory_ptr, flags_empty);
                required_extra_bytes_to_record--;
            }
        }
        if (stop_flag)
            break;
    }
    delete[] buffer;
    
    scan_progress = 1.0;
    return true;
}



bool
Scanner::scan_next(const matches_t& matches_source,
                   matches_t& matches_sink,
                   const scan_data_type_t& data_type,
                   scan_match_type_t match_type,
                   const uservalue_t *uservalue)
{
    using namespace std;
    
    if (!sm_choose_scanroutine(data_type, match_type, uservalue, 0)) {
        printf("unsupported scan for current data type.\n");
        return false;
    }
    
    unsigned long bytes_scanned = 0;
    unsigned long total_scan_bytes = 0;
    size_t bytes_at_next_sample;
    size_t bytes_per_sample;
    
    
    
//    size_t reading_iterator = 0;
//    int required_extra_bytes_to_record = 0;
//    matches_sink.matches_size = 0;
//    scan_progress = 0.0;
//    stop_flag = false;
//    
//    for(swath_t &swath : matches_source.swaths) {
//        unsigned int match_length = 0;
//        const mem64_t *memory_ptr;
//        size_t memlength;
//        uint16_t checkflags = flags_empty;
//        
//        uint16_t old_flags = swath.data[reading_iterator].flags;
//        size_t old_length = flags_to_memlength(data_type, old_flags);
//        uintptr_t address = swath.base_address + reading_iterator;
//        
//        /* read value from this address */
//        if (UNLIKELY(sm_peekdata((void *) address, old_length, &memory_ptr, &memlength) == false)) {
//            /* If we can't look at the data here, just abort the whole recording, something bad happened */
//            required_extra_bytes_to_record = 0;
//        }
//        /* Test only valid old matches */
//        else if (old_flags != flags_empty)  {
//            value_t old_val = data_to_val_aux(reading_swath_index, reading_iterator, reading_swath.number_of_bytes);
//            memlength = old_length < memlength ? old_length : memlength;
//            
//            checkflags = flags_empty;
//            
//            match_length = (*sm_scan_routine)(memory_ptr, memlength, &old_val, uservalue, checkflags);
//        }
//        
//        if (match_length > 0) {
//            assert(match_length <= memlength);
//            
//            /* Still a candidate. Write data.
//               - We can get away with overwriting in the same array because it is guaranteed to take up the same number of bytes or fewer,
//                 and because we copied out the reading swath metadata already.
//               - We can get away with assuming that the pointers will stay valid,
//                 because as we never add more data to the array than there was before, it will not reallocate. */
//            matches_sink.add_element(address, memory_ptr, checkflags);
//            matches_sink.matches_size++;
//            required_extra_bytes_to_record = match_length - 1;
//        }
//        else if (required_extra_bytes_to_record) {
//            matches_sink.add_element(address, memory_ptr, flags_empty);
//            required_extra_bytes_to_record--;
//        }
//        
//        ++bytes_scanned;
//        
//        /* go on to the next one... */
//        ++reading_iterator;
//        if (reading_iterator >= reading_swath.number_of_bytes)
//        {
//            reading_swath_index = (swath_t *)
//                    (&reading_swath_index->data[reading_swath.number_of_bytes]);
//            reading_swath = *reading_swath_index;
//            reading_iterator = 0;
//            required_extra_bytes_to_record = 0; /* just in case */
//        }
//    }
//    
    return true;
}



bool
Scanner::scan_reset()
{
//    snapshots.clear();
//    matches.clear();
    return true;
}
