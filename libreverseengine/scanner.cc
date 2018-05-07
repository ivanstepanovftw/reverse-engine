/*
    This file is part of Reverse Engine.

    

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

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
            uservalue[0].string_value = text;
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


#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.u8  == uservalue[0].u8 )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.u16 == uservalue[0].u16)) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.u32 == uservalue[0].u32)) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.u64 == uservalue[0].u64)) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.f32 == uservalue[0].f32)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.f64 == uservalue[0].f64)) m.flags |= (flag_f64b);


bool
Scanner::scan(matches_t& matches_sink,
              const scan_data_type_t& data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t& match_type)
{
    using namespace std;
    
    scan_progress = 0.0;
    stop_flag = false;
    
    if (!matches_sink.snapshot_mf.is_open()) {
        clog<<"сканируем на лету"<<endl;
        constexpr size_t RESERVED = sizeof(mem64_t) - 1;
        constexpr size_t CHECK_SIZE = 32 * 1024 * 1024;        // 32 MiB = 0x2000000
        constexpr size_t BUFFER_SIZE = CHECK_SIZE + RESERVED;  // 7 bytes reserved at end
        
        char *buffer_begin = new char[BUFFER_SIZE];
        char *buffer_end = buffer_begin + BUFFER_SIZE;
        char *buffer = buffer_begin;
        bzero(buffer_begin, BUFFER_SIZE);
        
        match_t m;
        ssize_t copied;
        
        
        for(region_t region : handle->regions) {
            m.address = region.address;
            /** пока регион слишком большой */
            while (region.size - RESERVED > CHECK_SIZE) {
                /** читаем его по чанкам */
                copied = handle->read(buffer_begin, m.address, BUFFER_SIZE);
                if (copied < 0) {
//                    clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
//                    if (!handle->is_running())
//                        throw invalid_argument("process not running");
                    goto next_region;
                } else if (copied != BUFFER_SIZE) {
                    clog<<"warning: region: "<<region<<", requested: "<<HEX(BUFFER_SIZE)<<", copied "<<HEX(copied)<<endl;
                    region.size = static_cast<uintptr_t>(copied);
                    goto asdasd;
                }
                
                /** сканируем */
                buffer = buffer_begin;
                buffer_end = buffer_begin + copied - RESERVED;
                
                while (buffer < buffer_end) {
                    m.memory = *reinterpret_cast<mem64_t *>(buffer);
                    FLAGS_CHECK_CODE
                    if (__glibc_unlikely(m.flags > 0))
                        matches_sink.add_element(m);
                    m.address += step;
                    buffer += step;
                }
                
                /** сообщаем, что пропустили несколько байтов */
                region.size -= (copied - RESERVED);
            }
            
            clog<<"readin122g"<<endl;
            /** сейчас читаем либо последний чанк, либо маленький регион */
            bzero(buffer_begin + region.size, RESERVED);
            copied = handle->read(buffer_begin, m.address, region.size);
            if (copied < 0) {
//                clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
//                if (!handle->is_running())
//                    throw invalid_argument("process not running");
                goto next_region;
            } else if (copied != region.size) {
                clog<<"warning: region: "<<region<<", requested: "<<HEX(region.size)<<", copied "<<HEX(copied)<<endl;
                region.size = static_cast<uintptr_t>(copied);
            }
            
asdasd:;    /** сканируем, не забывая исключать invalid flags */
            buffer = buffer_begin;
            buffer_end = buffer_begin + copied - RESERVED;
            while (buffer < buffer_end) {
                m.memory = *reinterpret_cast<mem64_t *>(buffer);
                FLAGS_CHECK_CODE
                if (__glibc_unlikely(m.flags > 0))
                    matches_sink.add_element(m);
                m.address += step;
                buffer += step;
            }
        
            buffer_end += 4;
            while (buffer < buffer_end) {
                m.memory = *reinterpret_cast<mem64_t *>(buffer);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b);
                if (__glibc_unlikely(m.flags > 0))
                    matches_sink.add_element(m);
                m.address += step;
                buffer += step;
            }
        
            buffer_end += 2;
            while (buffer < buffer_end) {
                m.memory = *reinterpret_cast<mem64_t *>(buffer);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b | flags_32b);
                if (__glibc_unlikely(m.flags > 0))
                    matches_sink.add_element(m);
                m.address += step;
                buffer += step;
            }
        
            buffer_end += 1;
            while (buffer < buffer_end) {
                m.memory = *reinterpret_cast<mem64_t *>(buffer);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b | flags_32b | flags_16b);
                if (__glibc_unlikely(m.flags > 0))
                    matches_sink.add_element(m);
                m.address += step;
                buffer += step;
            }
next_region:;
        }
        clog<<"readed: "<<matches_sink.matches_size<<endl;
    
        scan_progress = 0.0;
        stop_flag = false;
    }
    else {
        clog<<"сканируем снепшот на диске"<<endl;
        constexpr size_t RESERVED = sizeof(mem64_t) - 1;
        char *snapshot_begin = matches_sink.snapshot_mf.begin();
        char *snapshot_end = matches_sink.snapshot_mf.end();
        char *snapshot = snapshot_begin;
        char *region_end;
    
//    bio::mapped_file_params flags_mf_(matches_sink.path+".flg");
//    flags_mf_.flags         = bio::mapped_file::mapmode::readwrite;
//    flags_mf_.length        = matches_sink.snapshot_mf.size()*sizeof(match_t::flags);
//    flags_mf_.new_file_size = matches_sink.snapshot_mf.size()*sizeof(match_t::flags);
//    if (matches_sink.flags_mf.is_open())
//        matches_sink.flags_mf.close();
//    matches_sink.flags_mf.open(flags_mf_);
//    if (!matches_sink.flags_mf.is_open())
//        throw invalid_argument("cant open '"+matches_sink.path+".flg"+"'");
    
        std::vector<uint16_t> flags;
        match_t m;
        region_t region;


#define NUMBER_SCAN(FLAGS_CHECK_CODE)
        while (snapshot < snapshot_end) {
            /** сначала десериализуем регион, а именно первые два поля. TODO 334 */
            memcpy(&region,
                   snapshot,
                   2 * sizeof(region.address));
            snapshot += 2 * sizeof(region.address);
        
            /** сканируем, не забывая исключать invalid flags */
            m.address = region.address;
            region_end = snapshot + region.size - RESERVED;
        
            while (snapshot < region_end) {
                m.memory = *reinterpret_cast<mem64_t *>(snapshot);
                FLAGS_CHECK_CODE
                if (m.flags)
                    flags.emplace_back(m.flags);
                m.address += step;
                snapshot += step;
            }
        
            region_end += 4;
            while (snapshot < region_end) {
                m.memory = *reinterpret_cast<mem64_t *>(snapshot);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b);
                if (m.flags)
                    flags.emplace_back(m.flags);
                m.address += step;
                snapshot += step;
            }
        
            region_end += 2;
            while (snapshot < region_end) {
                m.memory = *reinterpret_cast<mem64_t *>(snapshot);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b | flags_32b);
                if (m.flags)
                    flags.emplace_back(m.flags);
                m.address += step;
                snapshot += step;
            }
        
            region_end += 1;
            while (snapshot < region_end) {
                m.memory = *reinterpret_cast<mem64_t *>(snapshot);
                FLAGS_CHECK_CODE
                m.flags &= ~(flags_64b | flags_32b | flags_16b);
                if (m.flags)
                    flags.emplace_back(m.flags);
                m.address += step;
                snapshot += step;
            }
        }
        clog<<"res: "<<flags.size()<<endl;
        flags.clear();
    }
    if (data_type == BYTEARRAY) {
        clog<<"not supported"<<endl;
    }
    else if (data_type == STRING) {
        clog<<"not supported"<<endl;
    } else {
        switch(match_type) {
            case MATCHANY:
                NUMBER_SCAN(
                        m.flags = flags_all;
                )
                break;
            case MATCHEQUALTO:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].u8  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].u16 )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].u32 )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].u64 )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].f32)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].f64)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHNOTEQUALTO:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   != uservalue[0].u8  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  != uservalue[0].u16 )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  != uservalue[0].u32 )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  != uservalue[0].u64 )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value != uservalue[0].f32)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value != uservalue[0].f64)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHGREATERTHAN:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   >  uservalue[0].u8  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  >  uservalue[0].u16 )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  >  uservalue[0].u32 )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  >  uservalue[0].u64 )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value >  uservalue[0].f32)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value >  uservalue[0].f64)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHLESSTHAN:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   <  uservalue[0].u8  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  <  uservalue[0].u16 )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  <  uservalue[0].u32 )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  <  uservalue[0].u64 )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value <  uservalue[0].f32)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value <  uservalue[0].f64)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHRANGE:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (uservalue[0].u8   <= m.memory.uint8_value   ) && (m.memory.uint8_value   >= uservalue[1].u8  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (uservalue[0].u16  <= m.memory.uint16_value  ) && (m.memory.uint16_value  >= uservalue[1].u16 )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (uservalue[0].u32  <= m.memory.uint32_value  ) && (m.memory.uint32_value  >= uservalue[1].u32 )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (uservalue[0].u64  <= m.memory.uint64_value  ) && (m.memory.uint64_value  >= uservalue[1].u64 )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (uservalue[0].f32 <= m.memory.float32_value ) && (m.memory.float32_value >= uservalue[1].f32)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (uservalue[0].f64 <= m.memory.float64_value ) && (m.memory.float64_value >= uservalue[1].f64)) m.flags |= (flag_f64b);
                )
                break;
            default:
                clog<<"error: only scan supported"<<endl;
        }
    }
    
    scan_progress = 1.0;
    return true;
}



bool
Scanner::flags_compare(const matches_t& matches_source,
                       const matches_t& matches_sink)
{
    scan_progress = 0.0;
    stop_flag = false;
    
    uint16_t *flags_source_begin = reinterpret_cast<uint16_t *>(matches_source.flags_mf.begin());
    uint16_t *flags_source_end = reinterpret_cast<uint16_t *>(matches_source.flags_mf.end());
    uint16_t *flags_source = flags_source_begin;
    
    uint16_t *flags_sink_begin = reinterpret_cast<uint16_t *>(matches_sink.flags_mf.begin());
    uint16_t *flags_sink_end = reinterpret_cast<uint16_t *>(matches_sink.flags_mf.end());
    uint16_t *flags_sink = flags_sink_begin;
    
    while(flags_sink < flags_sink_end) {
        *flags_sink = *flags_sink & *flags_source;
        flags_source++;
        flags_sink++;
    }
    
    scan_progress = 1.0;
    return true;
}




bool
Scanner::scan_reset()
{
//    snapshots.clear();
//    matches.clear();
    return true;
}
