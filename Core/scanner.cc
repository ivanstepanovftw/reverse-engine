//
// Created by root on 09.03.18.
//

#include <bitset>
#include <chrono>
#include "scanner.hh"
#include "value.hh"

using namespace std;
using namespace std::chrono;

//bool
//Scanner::sm_checkmatches(scan_match_type_t match_type,
//                         const uservalue_t *uservalue)
//{
//    matches_and_old_values_swath *reading_swath_index = vars->matches->swaths;
//    matches_and_old_values_swath reading_swath = *reading_swath_index;
//    
//    unsigned long bytes_scanned = 0;
//    unsigned long total_scan_bytes = 0;
//    matches_and_old_values_swath *tmp_swath_index = reading_swath_index;
//    unsigned int samples_remaining = NUM_SAMPLES;
//    unsigned int samples_to_dot = SAMPLES_PER_DOT;
//    size_t bytes_at_next_sample;
//    size_t bytes_per_sample;
//    
//    if (sm_choose_scanroutine(vars->options.scan_data_type, match_type, uservalue,
//                              vars->options.reverse_endianness) == false) {
//        printf("unsupported scan for current data type.\n");
//        return false;
//    }
//    
//    while (tmp_swath_index->number_of_bytes) {
//        total_scan_bytes += tmp_swath_index->number_of_bytes;
//        tmp_swath_index = (matches_and_old_values_swath * )(
//                &tmp_swath_index->data[tmp_swath_index->number_of_bytes]);
//    }
//    bytes_per_sample = total_scan_bytes / NUM_SAMPLES;
//    bytes_at_next_sample = bytes_per_sample;
//    /* for user, just print the first dot */
//    print_a_dot();
//    
//    size_t reading_iterator = 0;
//    matches_and_old_values_swath *writing_swath_index = vars->matches->swaths;
//    writing_swath_index->first_byte_in_child = NULL;
//    writing_swath_index->number_of_bytes = 0;
//    
//    int required_extra_bytes_to_record = 0;
//    vars->num_matches = 0;
//    vars->scan_progress = 0.0;
//    vars->stop_flag = false;
//    
//    /* stop and attach to the target */
//    if (sm_attach(vars->target) == false)
//        return false;
//    
//    INTERRUPTABLESCAN();
//    
//    while (reading_swath.first_byte_in_child) {
//        unsigned int match_length = 0;
//        const mem64_t *memory_ptr;
//        size_t memlength;
//        match_flags checkflags;
//        
//        match_flags old_flags = reading_swath_index->data[reading_iterator].match_info;
//        unsigned int old_length = flags_to_memlength(vars->options.scan_data_type, old_flags);
//        void *address = reading_swath.first_byte_in_child + reading_iterator;
//        
//        /* read value from this address */
//        if (UNLIKELY(sm_peekdata(address, old_length, &memory_ptr, &memlength) == false)) {
//            /* If we can't look at the data here, just abort the whole recording, something bad happened */
//            required_extra_bytes_to_record = 0;
//        } else if (old_flags != flags_empty) /* Test only valid old matches */
//        {
//            value_t old_val = data_to_val_aux(reading_swath_index, reading_iterator, reading_swath.number_of_bytes);
//            memlength = old_length < memlength ? old_length : memlength;
//            
//            checkflags = flags_empty;
//            
//            match_length = (*sm_scan_routine)(memory_ptr, memlength, &old_val, uservalue, &checkflags);
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
//            
//            writing_swath_index = add_element(&(vars->matches), writing_swath_index, address,
//                                              get_u8b(memory_ptr), checkflags);
//            
//            ++vars->num_matches;
//            
//            required_extra_bytes_to_record = match_length - 1;
//        } else if (required_extra_bytes_to_record) {
//            writing_swath_index = add_element(&(vars->matches), writing_swath_index, address,
//                                              get_u8b(memory_ptr), flags_empty);
//            --required_extra_bytes_to_record;
//        }
//        
//        if (UNLIKELY(bytes_scanned >= bytes_at_next_sample)) {
//            bytes_at_next_sample += bytes_per_sample;
//            /* handle rounding */
//            if (LIKELY(--samples_remaining > 0)) {
//                /* for front-end, update percentage */
//                vars->scan_progress += PROGRESS_PER_SAMPLE;
//                if (UNLIKELY(--samples_to_dot == 0)) {
//                    samples_to_dot = SAMPLES_PER_DOT;
//                    /* for user, just print a dot */
//                    print_a_dot();
//                }
//                /* stop scanning if asked to */
//                if (vars->stop_flag) {
//                    printf("\n");
//                    break;
//                }
//            }
//        }
//        ++bytes_scanned;
//        
//        /* go on to the next one... */
//        ++reading_iterator;
//        if (reading_iterator >= reading_swath.number_of_bytes) {
//            reading_swath_index = (matches_and_old_values_swath * )
//                    (&reading_swath_index->data[reading_swath.number_of_bytes]);
//            reading_swath = *reading_swath_index;
//            reading_iterator = 0;
//            required_extra_bytes_to_record = 0; /* just in case */
//        }
//    }
//    
//    if (!(vars->matches = null_terminate(vars->matches, writing_swath_index))) {
//        printf("memory allocation error while reducing matches-array size\n");
//        return false;
//    }
//    
//    this->scan_progress = 1.;
//    printf("we currently have %ld matches.\n", vars->num_matches);
//    return true;
//}


bool
Scanner::parse(scan_data_type_t data_type, const char *ustr, scan_match_type_t *match_type, uservalue_t *vals) {
    switch (data_type) {
        case BYTEARRAY:
            if (!parse_uservalue_bytearray(ustr, &vals[0])) {
                return false;
            }
            return true;
        case STRING:
            vals[0].string_value = ustr;
            vals[0].flags = flags_max;
            return true;
        default:
            char *pattern = const_cast<char *>(ustr);
            char *sign;
            // ╔════════════╤═══════╤═══════════════════════╗
            // ║ C operator │ Alias │ Description           ║
            // ╠════════════╪═══════╪═══════════════════════╣
            // ║            │ ?     │ unknown initial value ║
            // ║ == N       │ N     │ exactly N             ║
            // ║ != N       │       │ not N                 ║
            // ║ > N        │       │ greater than N        ║
            // ║ < N        │       │ less than N           ║
            // ║            │ N..M  │ range                 ║
            // ╟────────────┼───────┼───────────────────────╢
            // ║ ==         │       │ not changed           ║
            // ║ !=         │       │ changed               ║
            // ║ ++         │ >     │ increased             ║
            // ║ --         │ <     │ decreased             ║
            // ║ += N       │       │ increased by N        ║
            // ║ -= N       │       │ decreased by N        ║
            // ╚════════════╧═══════╧═══════════════════════╝
            while (isspace(*pattern)) pattern++;
            
            sign = strstr(pattern, "?");
            if (sign) {
                clog<<"?"<<endl;
                sign += 1;
                while (isspace(*sign)) sign++;
                if (*sign != '\0') return false;
                *match_type = MATCHANY;
                goto valid_number;
            }
            
            sign = strstr(pattern, "==");
            if (sign) {
                clog<<"=="<<endl;
                clog<<"sign: "<<sign<<endl;
                sign += 2;
                clog<<"sign+=2: sign: "<<sign<<endl;
                while (isspace(*sign)) sign++;
                clog<<"isspace(sign): "<<sign<<endl;
                if (!parse_uservalue_number(sign, &vals[0])) {
                clog<<"cant parse uservalue (sign): "<<sign<<endl;
                    
                    while (isspace(*sign)) sign++;
                    if (*sign != '\0') return false;
                    *match_type = MATCHNOTCHANGED;
                    goto valid_number;
                }
                *match_type = MATCHEQUALTO;
                goto valid_number;
            }
            
            sign = strstr(pattern, "!=");
            if (sign) {
            clog<<"!="<<endl;
                sign += 2;
                while (isspace(*sign)) sign++;
                if (!parse_uservalue_number(sign, &vals[0])) {
                    while (isspace(*sign)) sign++;
                    if (*sign != '\0') return false;
                    *match_type = MATCHCHANGED;
                    goto valid_number;
                }
                *match_type = MATCHNOTEQUALTO;
                goto valid_number;
            }
            
            sign = strstr(pattern, ">");
            if (sign) {
                clog<<">"<<endl;
                sign += 1;
                while (isspace(*sign)) sign++;
                if (!parse_uservalue_number(sign, &vals[0])) {
                    while (isspace(*sign)) sign++;
                    if (*sign != '\0') return false;
                    *match_type = MATCHINCREASED;
                    goto valid_number;
                }
                *match_type = MATCHGREATERTHAN;
                goto valid_number;
            }
            
            sign = strstr(pattern, "<");
            if (sign) {
                clog<<"<"<<endl;
                sign += 1;
                while (isspace(*sign)) sign++;
                if (!parse_uservalue_number(sign, &vals[0])) {
                    while (isspace(*sign)) sign++;
                    if (*sign != '\0') return false;
                    *match_type = MATCHDECREASED;
                    goto valid_number;
                }
                *match_type = MATCHLESSTHAN;
                goto valid_number;
            }
        
            sign = strstr(pattern, "++");
            if (sign) {
                clog<<"++"<<endl;
                sign += 2;
                while (isspace(*sign)) sign++;
                if (*sign != '\0') return false;
                *match_type = MATCHINCREASED;
                goto valid_number;
            }
        
            sign = strstr(pattern, "--");
            if (sign) {
                clog<<"--"<<endl;
                sign += 2;
                while (isspace(*sign)) sign++;
                if (*sign != '\0') return false;
                *match_type = MATCHDECREASED;
                goto valid_number;
            }
            
            sign = strstr(pattern, "+=");
            if (sign) {
                clog<<"+="<<endl;
                sign += 2;
                while (isspace(*sign)) sign++;
                if (!parse_uservalue_number(sign, &vals[0])) {
                    return false;
                }
                *match_type = MATCHINCREASEDBY;
                goto valid_number;
            }
            
            sign = strstr(pattern, "-=");
            if (sign) {
                clog<<"-="<<endl;
                sign += 2;
                while (isspace(*sign)) sign++;
                if (!parse_uservalue_number(sign, &vals[0])) {
                    return false;
                }
                *match_type = MATCHDECREASEDBY;
                goto valid_number;
            }
            
            sign = strstr(pattern, "..");
            if (sign) {
                /// Parse last number n..M
                sign += 2;
                if (!parse_uservalue_number(sign, &vals[1]))
                    return false;
                
                /// Parse first number N..m
                char *rev = pattern;
                reverse(rev, rev+strlen(rev));
                rev = strstr(pattern, "..") + 2;
                if (!parse_uservalue_number(rev, &vals[0]))
                    return false;
                
                /// Check that the range is nonempty
                if (vals[0].float64_value > vals[1].float64_value) {
                    clog<<"Empty range"<<endl;
                    return false;
                }
                
                /// Store the bitwise AND of both flags in the first value
                vals[0].flags = static_cast<match_flags>(vals[0].flags & vals[1].flags);
                *match_type = MATCHRANGE;
                goto valid_number;
            }
            
            if (!parse_uservalue_number(pattern, &vals[0])) {
                return false;
            }
            *match_type = MATCHEQUALTO;
            goto valid_number;
    }
    
valid_number:;
    vals[0].flags = static_cast<match_flags>(vals[0].flags & scan_data_type_to_flags[data_type]);
    vals[1].flags = static_cast<match_flags>(vals[1].flags & scan_data_type_to_flags[data_type]);
    return true;
}


bool
Scanner::first_scan(scan_data_type_t data_type, const char *ustr)
{
    uservalue_t vals[2];
    scan_match_type_t match_type;
    if (!this->parse(data_type, ustr, &match_type, vals)) {
        return false;
    }
    
    if (!this->handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }
    
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : this->handle->regions) {
        if (region.start > region.end) {
            clog<<"error: invalid region: empty range: "<<region<<endl;
            return false;
        }
        total_scan_bytes += region.end - region.start;
    }
    
    if (total_scan_bytes == 0) {
        if (this->handle->regions.size() == 0)
            clog<<"error: no regions defined, perhaps you deleted them all?"<<endl;
        else
            clog<<"error: "<<this->handle->regions.size()<<" regions defined, but each have 0 bytes"<<endl;
        return false;
    }
// Release
//    Searching took 0.00681768 seconds.
//    We currently have 333305 matches.
// Debug
//    Searching took 0.0475823 seconds.
//    We currently have 333305 matches.
//    Searching took 0.040084 seconds.
//    We currently have 333305 matches.
    
    this->matches.clear();
    this->scan_progress = 0.0;
    this->stop_flag = false;
    auto time_point = high_resolution_clock::now();
    
    switch (data_type) {
        case BYTEARRAY:
            
            break;
        case STRING:
            
            break;
        default:
            match_flags flags;
            switch (match_type) {
                case MATCHANY:
                    this->matches.reserve(total_scan_bytes);
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, vals[0].flags);
                        }
                    break;
                case MATCHEQUALTO:
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            flags = flags_empty;
                            if ((vals[0].flags & flag_s64b) && i_memory_ptr->int64_value   == vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
                            if ((vals[0].flags & flag_s32b) && i_memory_ptr->int32_value   == vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
                            if ((vals[0].flags & flag_s16b) && i_memory_ptr->int16_value   == vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
                            if ((vals[0].flags & flag_s8b)  && i_memory_ptr->int8_value    == vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
                            if ((vals[0].flags & flag_u64b) && i_memory_ptr->uint64_value  == vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
                            if ((vals[0].flags & flag_u32b) && i_memory_ptr->uint32_value  == vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
                            if ((vals[0].flags & flag_u16b) && i_memory_ptr->uint16_value  == vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
                            if ((vals[0].flags & flag_u8b)  && i_memory_ptr->uint8_value   == vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
                            if ((vals[0].flags & flag_f64b) && i_memory_ptr->float64_value == vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
                            if ((vals[0].flags & flag_f32b) && i_memory_ptr->float32_value == vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
                            if (this->stop_flag) return false;
                        }
                    break;
                case MATCHNOTEQUALTO:
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            flags = flags_empty;
                            if ((vals[0].flags & flag_s64b) && i_memory_ptr->int64_value   != vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
                            if ((vals[0].flags & flag_s32b) && i_memory_ptr->int32_value   != vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
                            if ((vals[0].flags & flag_s16b) && i_memory_ptr->int16_value   != vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
                            if ((vals[0].flags & flag_s8b)  && i_memory_ptr->int8_value    != vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
                            if ((vals[0].flags & flag_u64b) && i_memory_ptr->uint64_value  != vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
                            if ((vals[0].flags & flag_u32b) && i_memory_ptr->uint32_value  != vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
                            if ((vals[0].flags & flag_u16b) && i_memory_ptr->uint16_value  != vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
                            if ((vals[0].flags & flag_u8b)  && i_memory_ptr->uint8_value   != vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
                            if ((vals[0].flags & flag_f64b) && i_memory_ptr->float64_value != vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
                            if ((vals[0].flags & flag_f32b) && i_memory_ptr->float32_value != vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
                            if (this->stop_flag) return false;
                        }
                    break;
                case MATCHGREATERTHAN:
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            flags = flags_empty;
                            if ((vals[0].flags & flag_s64b) && i_memory_ptr->int64_value   >  vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
                            if ((vals[0].flags & flag_s32b) && i_memory_ptr->int32_value   >  vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
                            if ((vals[0].flags & flag_s16b) && i_memory_ptr->int16_value   >  vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
                            if ((vals[0].flags & flag_s8b)  && i_memory_ptr->int8_value    >  vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
                            if ((vals[0].flags & flag_u64b) && i_memory_ptr->uint64_value  >  vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
                            if ((vals[0].flags & flag_u32b) && i_memory_ptr->uint32_value  >  vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
                            if ((vals[0].flags & flag_u16b) && i_memory_ptr->uint16_value  >  vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
                            if ((vals[0].flags & flag_u8b)  && i_memory_ptr->uint8_value   >  vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
                            if ((vals[0].flags & flag_f64b) && i_memory_ptr->float64_value >  vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
                            if ((vals[0].flags & flag_f32b) && i_memory_ptr->float32_value >  vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
                            if (this->stop_flag) return false;
                        }
                    break;
                case MATCHLESSTHAN:
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            flags = flags_empty;
                            if ((vals[0].flags & flag_s64b) && i_memory_ptr->int64_value   <  vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
                            if ((vals[0].flags & flag_s32b) && i_memory_ptr->int32_value   <  vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
                            if ((vals[0].flags & flag_s16b) && i_memory_ptr->int16_value   <  vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
                            if ((vals[0].flags & flag_s8b)  && i_memory_ptr->int8_value    <  vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
                            if ((vals[0].flags & flag_u64b) && i_memory_ptr->uint64_value  <  vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
                            if ((vals[0].flags & flag_u32b) && i_memory_ptr->uint32_value  <  vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
                            if ((vals[0].flags & flag_u16b) && i_memory_ptr->uint16_value  <  vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
                            if ((vals[0].flags & flag_u8b)  && i_memory_ptr->uint8_value   <  vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
                            if ((vals[0].flags & flag_f64b) && i_memory_ptr->float64_value <  vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
                            if ((vals[0].flags & flag_f32b) && i_memory_ptr->float32_value <  vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
                            if (this->stop_flag) return false;
                        }
                    break;
                case MATCHRANGE:
                    for(;i_nextRegion();)
                        for(;i_nextMemory();) {
                            flags = flags_empty;
                            if ((vals[0].flags & flag_s64b) && vals[0].int64_value   <= i_memory_ptr->int64_value   || i_memory_ptr->int64_value   <= vals[1].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
                            if ((vals[0].flags & flag_s32b) && vals[0].int32_value   <= i_memory_ptr->int32_value   || i_memory_ptr->int32_value   <= vals[1].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
                            if ((vals[0].flags & flag_s16b) && vals[0].int16_value   <= i_memory_ptr->int16_value   || i_memory_ptr->int16_value   <= vals[1].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
                            if ((vals[0].flags & flag_s8b)  && vals[0].int8_value    <= i_memory_ptr->int8_value    || i_memory_ptr->int8_value    <= vals[1].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
                            if ((vals[0].flags & flag_u64b) && vals[0].uint64_value  <= i_memory_ptr->uint64_value  || i_memory_ptr->uint64_value  <= vals[1].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
                            if ((vals[0].flags & flag_u32b) && vals[0].uint32_value  <= i_memory_ptr->uint32_value  || i_memory_ptr->uint32_value  <= vals[1].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
                            if ((vals[0].flags & flag_u16b) && vals[0].uint16_value  <= i_memory_ptr->uint16_value  || i_memory_ptr->uint16_value  <= vals[1].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
                            if ((vals[0].flags & flag_u8b)  && vals[0].uint8_value   <= i_memory_ptr->uint8_value   || i_memory_ptr->uint8_value   <= vals[1].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
                            if ((vals[0].flags & flag_f64b) && vals[0].float64_value <= i_memory_ptr->float64_value || i_memory_ptr->float64_value <= vals[1].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
                            if ((vals[0].flags & flag_f32b) && vals[0].float32_value <= i_memory_ptr->float32_value || i_memory_ptr->float32_value <= vals[1].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
                            if (this->stop_flag) return false;
                        }
                    break;
                case MATCHUPDATE:
                    clog<<"MATCHUPDATE"<<endl;
                    break;
                case MATCHNOTCHANGED:
                    clog<<"MATCHNOTCHANGED"<<endl;
                    break;
                case MATCHCHANGED:
                    clog<<"MATCHCHANGED"<<endl;
                    break;
                case MATCHINCREASED:
                    clog<<"MATCHINCREASED"<<endl;
                    break;
                case MATCHDECREASED:
                    clog<<"MATCHDECREASED"<<endl;
                    break;
                case MATCHINCREASEDBY:
                    clog<<"MATCHINCREASEDBY"<<endl;
                    break;
                case MATCHDECREASEDBY:
                    clog<<"MATCHDECREASEDBY"<<endl;
                    break;
            }
    }
    
    clog<<"Done: "<<this->matches.size()<<" matches, in: "<<(duration_cast<duration<double>>(high_resolution_clock::now() - time_point)).count()<<" second."<<endl;
    
    this->scan_progress = 1.0;

    return true;
}



bool
Scanner::next_scan(scan_data_type_t data_type, char *ustr)
{
    uservalue_t vals[2];
    scan_match_type_t match_type = MATCHEQUALTO;
    if (!parse(data_type, ustr, &match_type, vals)) {
        return false;
    }

    if (!handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }

    if (matches.size() == 0) {
        printf("there are currently no matches.\n");
        return false;
    }
    
    this->scan_progress = 0.0;
    this->stop_flag = false;
    auto time_point = high_resolution_clock::now();
    
    switch (data_type) {
        case BYTEARRAY:
            
            break;
        case STRING:
            
            break;
        default: ;
//            switch (match_type) {
//                case MATCHEQUALTO:
//                    for(;i_nextRegion();)
//                        for(;i_nextMemory();) {
//                            flags = flags_empty;
//                            if ((flags & flag_s64b) && i_memory_ptr->int64_value   == vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
//                            if ((flags & flag_s32b) && i_memory_ptr->int32_value   == vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
//                            if ((flags & flag_s16b) && i_memory_ptr->int16_value   == vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
//                            if ((flags & flag_s8b)  && i_memory_ptr->int8_value    == vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
//                            if ((flags & flag_u64b) && i_memory_ptr->uint64_value  == vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
//                            if ((flags & flag_u32b) && i_memory_ptr->uint32_value  == vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
//                            if ((flags & flag_u16b) && i_memory_ptr->uint16_value  == vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
//                            if ((flags & flag_u8b)  && i_memory_ptr->uint8_value   == vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
//                            if ((flags & flag_f64b) && i_memory_ptr->float64_value == vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
//                            if ((flags & flag_f32b) && i_memory_ptr->float32_value == vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
//                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
//                            if (this->stop_flag) return false;
//                        }
//                    break;
//                case MATCHNOTEQUALTO:
//                    for(;i_nextRegion();)
//                        for(;i_nextMemory();) {
//                            flags = flags_empty;
//                            if ((flags & flag_s64b) && i_memory_ptr->int64_value   != vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
//                            if ((flags & flag_s32b) && i_memory_ptr->int32_value   != vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
//                            if ((flags & flag_s16b) && i_memory_ptr->int16_value   != vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
//                            if ((flags & flag_s8b)  && i_memory_ptr->int8_value    != vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
//                            if ((flags & flag_u64b) && i_memory_ptr->uint64_value  != vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
//                            if ((flags & flag_u32b) && i_memory_ptr->uint32_value  != vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
//                            if ((flags & flag_u16b) && i_memory_ptr->uint16_value  != vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
//                            if ((flags & flag_u8b)  && i_memory_ptr->uint8_value   != vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
//                            if ((flags & flag_f64b) && i_memory_ptr->float64_value != vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
//                            if ((flags & flag_f32b) && i_memory_ptr->float32_value != vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
//                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
//                            if (this->stop_flag) return false;
//                        }
//                    break;
//                case MATCHGREATERTHAN:
//                    for(;i_nextRegion();)
//                        for(;i_nextMemory();) {
//                            flags = flags_empty;
//                            if ((flags & flag_s64b) && i_memory_ptr->int64_value   >  vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
//                            if ((flags & flag_s32b) && i_memory_ptr->int32_value   >  vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
//                            if ((flags & flag_s16b) && i_memory_ptr->int16_value   >  vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
//                            if ((flags & flag_s8b)  && i_memory_ptr->int8_value    >  vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
//                            if ((flags & flag_u64b) && i_memory_ptr->uint64_value  >  vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
//                            if ((flags & flag_u32b) && i_memory_ptr->uint32_value  >  vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
//                            if ((flags & flag_u16b) && i_memory_ptr->uint16_value  >  vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
//                            if ((flags & flag_u8b)  && i_memory_ptr->uint8_value   >  vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
//                            if ((flags & flag_f64b) && i_memory_ptr->float64_value >  vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
//                            if ((flags & flag_f32b) && i_memory_ptr->float32_value >  vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
//                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
//                            if (this->stop_flag) return false;
//                        }
//                    break;
//                case MATCHLESSTHAN:
//                    for(;i_nextRegion();)
//                        for(;i_nextMemory();) {
//                            flags = flags_empty;
//                            if ((flags & flag_s64b) && i_memory_ptr->int64_value   <  vals[0].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
//                            if ((flags & flag_s32b) && i_memory_ptr->int32_value   <  vals[0].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
//                            if ((flags & flag_s16b) && i_memory_ptr->int16_value   <  vals[0].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
//                            if ((flags & flag_s8b)  && i_memory_ptr->int8_value    <  vals[0].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
//                            if ((flags & flag_u64b) && i_memory_ptr->uint64_value  <  vals[0].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
//                            if ((flags & flag_u32b) && i_memory_ptr->uint32_value  <  vals[0].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
//                            if ((flags & flag_u16b) && i_memory_ptr->uint16_value  <  vals[0].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
//                            if ((flags & flag_u8b)  && i_memory_ptr->uint8_value   <  vals[0].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
//                            if ((flags & flag_f64b) && i_memory_ptr->float64_value <  vals[0].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
//                            if ((flags & flag_f32b) && i_memory_ptr->float32_value <  vals[0].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
//                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
//                            if (this->stop_flag) return false;
//                        }
//                    break;
//                case MATCHRANGE:
//                    for(;i_nextRegion();)
//                        for(;i_nextMemory();) {
//                            flags = flags_empty;
//                            if ((flags & flag_s64b) && vals[0].int64_value   <= i_memory_ptr->int64_value   || i_memory_ptr->int64_value   <= vals[1].int64_value)   flags = static_cast<match_flags>(flags | flag_s64b);
//                            if ((flags & flag_s32b) && vals[0].int32_value   <= i_memory_ptr->int32_value   || i_memory_ptr->int32_value   <= vals[1].int32_value)   flags = static_cast<match_flags>(flags | flag_s32b);
//                            if ((flags & flag_s16b) && vals[0].int16_value   <= i_memory_ptr->int16_value   || i_memory_ptr->int16_value   <= vals[1].int16_value)   flags = static_cast<match_flags>(flags | flag_s16b);
//                            if ((flags & flag_s8b)  && vals[0].int8_value    <= i_memory_ptr->int8_value    || i_memory_ptr->int8_value    <= vals[1].int8_value)    flags = static_cast<match_flags>(flags | flag_s8b);
//                            if ((flags & flag_u64b) && vals[0].uint64_value  <= i_memory_ptr->uint64_value  || i_memory_ptr->uint64_value  <= vals[1].uint64_value)  flags = static_cast<match_flags>(flags | flag_u64b);
//                            if ((flags & flag_u32b) && vals[0].uint32_value  <= i_memory_ptr->uint32_value  || i_memory_ptr->uint32_value  <= vals[1].uint32_value)  flags = static_cast<match_flags>(flags | flag_u32b);
//                            if ((flags & flag_u16b) && vals[0].uint16_value  <= i_memory_ptr->uint16_value  || i_memory_ptr->uint16_value  <= vals[1].uint16_value)  flags = static_cast<match_flags>(flags | flag_u16b);
//                            if ((flags & flag_u8b)  && vals[0].uint8_value   <= i_memory_ptr->uint8_value   || i_memory_ptr->uint8_value   <= vals[1].uint8_value)   flags = static_cast<match_flags>(flags | flag_u8b);
//                            if ((flags & flag_f64b) && vals[0].float64_value <= i_memory_ptr->float64_value || i_memory_ptr->float64_value <= vals[1].float64_value) flags = static_cast<match_flags>(flags | flag_f64b);
//                            if ((flags & flag_f32b) && vals[0].float32_value <= i_memory_ptr->float32_value || i_memory_ptr->float32_value <= vals[1].float32_value) flags = static_cast<match_flags>(flags | flag_f32b);
//                            if (flags) this->matches.emplace_back(i_region, i_offset, *i_memory_ptr, flags);
//                            if (this->stop_flag) return false;
//                        }
//                    break;
//                case MATCHUPDATE:
//                    clog<<"MATCHUPDATE"<<endl;
//                    break;
//                case MATCHNOTCHANGED:
//                    clog<<"MATCHNOTCHANGED"<<endl;
//                    break;
//                case MATCHCHANGED:
//                    clog<<"MATCHCHANGED"<<endl;
//                    break;
//                case MATCHINCREASED:
//                    clog<<"MATCHINCREASED"<<endl;
//                    break;
//                case MATCHDECREASED:
//                    clog<<"MATCHDECREASED"<<endl;
//                    break;
//                case MATCHINCREASEDBY:
//                    clog<<"MATCHINCREASEDBY"<<endl;
//                    break;
//                case MATCHDECREASEDBY:
//                    clog<<"MATCHDECREASEDBY"<<endl;
//                    break;
//            }
    }
    
    this->scan_progress = 1.0;
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
