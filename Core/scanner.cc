//
// Created by root on 09.03.18.
//

#include "scanner.hh"

namespace bio = boost::iostreams;

void
Scanner::string_to_uservalue(const scan_data_type_t &data_type, const string &text,
                             scan_match_type_t *match_type, uservalue_t *uservalue)
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
                if (uservalue[0].float64_value > uservalue[1].float64_value) {
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
Scanner::make_snapshot(const string &path)
{
    /// Allocate space
    if (handle->regions.size() == 0)
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
            clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
            if (!handle->is_running())
                throw invalid_argument("process not running");
            continue;
        } else if (copied != region.size) {
            clog<<"warning: region: "<<region<<", requested: "<<HEX(region.size)<<", copied "<<HEX(copied)<<endl;
            region.size = static_cast<uintptr_t>(copied);
        }
        memcpy(snapshot,
               &region.address,
               2*sizeof(region.address));
        snapshot += 2*sizeof(region.address) + copied;
    }
    snapshot_mf.resize(snapshot - snapshot_mf.data()); //fixme [high] тут +1 или не?
    
    return snapshot_mf;
}


#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);


bool
Scanner::scan(const scan_data_type_t data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t match_type)
/** сканирование без снепшота */
{
    scan_progress = 0.0;
    stop_flag = false;
    
    uintptr_t max_region_size = 0;
    for(const region_t& region : handle->regions)
        max_region_size = MAX(max_region_size, region.size);
    
    char *region_begin = new char[max_region_size];
    bzero(region_begin, max_region_size);
    char *region_cur;
    char *region_end;
    match_t m;
    ssize_t copied;

#define NUMBER_SCAN_NO_SNAPSHOT(FLAGS_CHECK_CODE)
    for(const region_t& region : handle->regions) {
        region_cur = region_begin;
        m.address = region.address;
        copied = handle->read(region_begin, m.address, region.size);
        if (copied < 0) {
            clog<<"error: "<<std::strerror(errno)<<", region: "<<region<<endl;
            if (!handle->is_running())
                throw invalid_argument("process not running");
            goto next_region;
        } else if (copied != region.size) {
            clog<<"warning: region: "<<region<<", requested: "<<HEX(region.size)<<", copied "<<HEX(copied)<<endl;
        }
        
        /** сканируем, не забывая исключать invalid flags */
        region_end = region_begin + copied - 7;
        while (region_cur < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(region_cur);
            FLAGS_CHECK_CODE
            if (m.flags)
                matches.append(m);
            m.address += step;
            region_cur += step;
        }
    
        region_end += 4;
        while (region_cur < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(region_cur);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            region_cur += step;
        }
    
        region_end += 2;
        while (region_cur < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(region_cur);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            region_cur += step;
        }
    
        region_end += 1;
        while (region_cur < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(region_cur);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b | flags_16b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            region_cur += step;
        }
        next_region:;
    }
    matches.flush();
    
//    
//    if (data_type == BYTEARRAY) {
//        clog<<"not supported"<<endl;
//    }
//    else if (data_type == STRING) {
//        clog<<"not supported"<<endl;
//    } else {
//        switch(match_type) {
//            case MATCHANY:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_all;
//                )
//                break;
//            case MATCHEQUALTO:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_empty;
//                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
//                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
//                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
//                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
//                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
//                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
//                )
//                break;
//            case MATCHNOTEQUALTO:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_empty;
//                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   != uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
//                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  != uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
//                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  != uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
//                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  != uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
//                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value != uservalue[0].float32_value)) m.flags |= (flag_f32b);
//                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value != uservalue[0].float64_value)) m.flags |= (flag_f64b);
//                )
//                break;
//            case MATCHGREATERTHAN:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_empty;
//                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   >  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
//                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  >  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
//                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  >  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
//                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  >  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
//                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value >  uservalue[0].float32_value)) m.flags |= (flag_f32b);
//                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value >  uservalue[0].float64_value)) m.flags |= (flag_f64b);
//                )
//                break;
//            case MATCHLESSTHAN:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_empty;
//                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   <  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
//                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  <  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
//                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  <  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
//                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  <  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
//                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value <  uservalue[0].float32_value)) m.flags |= (flag_f32b);
//                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value <  uservalue[0].float64_value)) m.flags |= (flag_f64b);
//                )
//                break;
//            case MATCHRANGE:
//                NUMBER_SCAN_NO_SNAPSHOT(
//                        m.flags = flags_empty;
//                        if ((uservalue[0].flags & flags_i8b ) && (uservalue[0].uint8_value   <= m.memory.uint8_value   ) && (m.memory.uint8_value   >= uservalue[1].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
//                        if ((uservalue[0].flags & flags_i16b) && (uservalue[0].uint16_value  <= m.memory.uint16_value  ) && (m.memory.uint16_value  >= uservalue[1].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
//                        if ((uservalue[0].flags & flags_i32b) && (uservalue[0].uint32_value  <= m.memory.uint32_value  ) && (m.memory.uint32_value  >= uservalue[1].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
//                        if ((uservalue[0].flags & flags_i64b) && (uservalue[0].uint64_value  <= m.memory.uint64_value  ) && (m.memory.uint64_value  >= uservalue[1].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
//                        if ((uservalue[0].flags & flag_f32b ) && (uservalue[0].float32_value <= m.memory.float32_value ) && (m.memory.float32_value >= uservalue[1].float32_value)) m.flags |= (flag_f32b);
//                        if ((uservalue[0].flags & flag_f64b ) && (uservalue[0].float64_value <= m.memory.float64_value ) && (m.memory.float64_value >= uservalue[1].float64_value)) m.flags |= (flag_f64b);
//                )
//                break;
//            default:
//                clog<<"error: only scan supported"<<endl;
//        }
//    }
    
    scan_progress = 1.0;
    return true;
}


bool
Scanner::scan(const bio::mapped_file& snapshot_mf,
              const scan_data_type_t data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t match_type)
/** сканирование с использованием снепшота */
{
    scan_progress = 0.0;
    stop_flag = false;
    
    constexpr size_t RESERVED = sizeof(mem64_t) - 1;
    char *snapshot_begin = snapshot_mf.data();
    char *snapshot_end = snapshot_begin + snapshot_mf.size();
    char *snapshot = snapshot_begin;
    char *region_end;
    
    match_t m;
    region_t region;
    
#define NUMBER_SCAN(FLAGS_CHECK_CODE)
    while(snapshot < snapshot_end) {
        /** сначала десериализуем регион, а именно первые два поля. TODO 334 */
        memcpy(&region,
               snapshot,
               2*sizeof(region.address));
        snapshot += 2*sizeof(region.address);
        
        /** сканируем, не забывая исключать invalid flags */
        m.address = region.address;
        region_end = snapshot + region.size - RESERVED;
        
        while (snapshot < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(snapshot);
            FLAGS_CHECK_CODE
            if (m.flags)
                matches.append(m);
            m.address += step;
            snapshot += step;
        }
        
        region_end += 4;
        while (snapshot < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(snapshot);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            snapshot += step;
        }
        
        region_end += 2;
        while (snapshot < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(snapshot);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            snapshot += step;
        }
    
        region_end += 1;
        while (snapshot < region_end) {
            m.memory = *reinterpret_cast<mem64_t *>(snapshot);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b | flags_16b);
            if (m.flags)
                matches.append(m);
            m.address += step;
            snapshot += step;
        }
    }
    matches.flush();
    
    
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
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHNOTEQUALTO:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   != uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  != uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  != uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  != uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value != uservalue[0].float32_value)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value != uservalue[0].float64_value)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHGREATERTHAN:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   >  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  >  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  >  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  >  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value >  uservalue[0].float32_value)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value >  uservalue[0].float64_value)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHLESSTHAN:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   <  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  <  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  <  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  <  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value <  uservalue[0].float32_value)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value <  uservalue[0].float64_value)) m.flags |= (flag_f64b);
                )
                break;
            case MATCHRANGE:
                NUMBER_SCAN(
                        m.flags = flags_empty;
                        if ((uservalue[0].flags & flags_i8b ) && (uservalue[0].uint8_value   <= m.memory.uint8_value   ) && (m.memory.uint8_value   >= uservalue[1].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                        if ((uservalue[0].flags & flags_i16b) && (uservalue[0].uint16_value  <= m.memory.uint16_value  ) && (m.memory.uint16_value  >= uservalue[1].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                        if ((uservalue[0].flags & flags_i32b) && (uservalue[0].uint32_value  <= m.memory.uint32_value  ) && (m.memory.uint32_value  >= uservalue[1].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                        if ((uservalue[0].flags & flags_i64b) && (uservalue[0].uint64_value  <= m.memory.uint64_value  ) && (m.memory.uint64_value  >= uservalue[1].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                        if ((uservalue[0].flags & flag_f32b ) && (uservalue[0].float32_value <= m.memory.float32_value ) && (m.memory.float32_value >= uservalue[1].float32_value)) m.flags |= (flag_f32b);
                        if ((uservalue[0].flags & flag_f64b ) && (uservalue[0].float64_value <= m.memory.float64_value ) && (m.memory.float64_value >= uservalue[1].float64_value)) m.flags |= (flag_f64b);
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
Scanner::scan_reset()
{
    scan_count = 0;
    matches.clear();
    return true;
}


//bool
//Scanner::update()
//{
//    if (matches.size()) {
//        if (!sm_checkmatches(MATCHUPDATE, NULL)) {
//            printf("failed to scan target address space.\n");
//            return false;
//        }
//    } else {
//        printf("cannot use that command without matches\n");
//        return false;
//    }
//    
//    return true;
//}
