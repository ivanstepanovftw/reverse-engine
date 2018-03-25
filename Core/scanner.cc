//
// Created by root on 09.03.18.
//

#include <bitset>
#include <chrono>
#include <cmath>
#include "scanner.hh"
#include "value.hh"

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

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


void
Scanner::string_to_uservalue(const scan_data_type_t &data_type, const string &text,
                             scan_match_type_t *match_type, uservalue_t *uservalue) {
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
            pattern.resize(static_cast<size_t>(std::distance(pattern.begin(), it)));
            
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

#define FLAG_ADD_IF_COMPARED_BY(FLAGS, MEM64, VS, VALUE1)                                                                    \
    if ((VALUE1.flags & flags_i8b ) && (MEM64->uint8_value   VS VALUE1.uint8_value  )) FLAGS |= (VALUE1.flags & flags_i8b);  \
    if ((VALUE1.flags & flags_i16b) && (MEM64->uint16_value  VS VALUE1.uint16_value )) FLAGS |= (VALUE1.flags & flags_i16b); \
    if ((VALUE1.flags & flags_i32b) && (MEM64->uint32_value  VS VALUE1.uint32_value )) FLAGS |= (VALUE1.flags & flags_i32b); \
    if ((VALUE1.flags & flags_i64b) && (MEM64->uint64_value  VS VALUE1.uint64_value )) FLAGS |= (VALUE1.flags & flags_i64b); \
    if ((VALUE1.flags & flag_f32b ) && (MEM64->float32_value VS VALUE1.float32_value)) FLAGS |= (flag_f32b);                 \
    if ((VALUE1.flags & flag_f64b ) && (MEM64->float64_value VS VALUE1.float64_value)) FLAGS |= (flag_f64b);

#define FLAG_REMOVE_IF_NOT_COMPARED_BY(FLAGS, MEM64, VS, VALUE1)                                                                                 \
    if ((FLAGS & flags_i8b ) &&                                 !(MEM64->uint8_value   VS VALUE1.uint8_value  )) FLAGS &= ~(FLAGS & flags_i8b);  \
    if ((FLAGS & flags_i16b) &&                                 !(MEM64->uint16_value  VS VALUE1.uint16_value )) FLAGS &= ~(FLAGS & flags_i16b); \
    if ((FLAGS & flags_i32b) &&                                 !(MEM64->uint32_value  VS VALUE1.uint32_value )) FLAGS &= ~(FLAGS & flags_i32b); \
    if ((FLAGS & flags_i64b) &&                                 !(MEM64->uint64_value  VS VALUE1.uint64_value )) FLAGS &= ~(FLAGS & flags_i64b); \
    if ((FLAGS & flag_f32b ) && !isnan(MEM64->float32_value) && !(MEM64->float32_value VS VALUE1.float32_value)) FLAGS &= ~(flag_f32b);          \
    if ((FLAGS & flag_f64b ) && !isnan(MEM64->float64_value) && !(MEM64->float64_value VS VALUE1.float64_value)) FLAGS &= ~(flag_f64b);

#define FLAG_ADD_IF_IN_RANGE(FLAGS, MEM64, VALUE1, VALUE2)                                                                                                                      \
    if ((VALUE1.flags & flags_i8b ) && (VALUE1.uint8_value   <= MEM64->uint8_value   ) && (MEM64->uint8_value   >= VALUE2.uint8_value  )) FLAGS |= (VALUE1.flags & flags_i8b);  \
    if ((VALUE1.flags & flags_i16b) && (VALUE1.uint16_value  <= MEM64->uint16_value  ) && (MEM64->uint16_value  >= VALUE2.uint16_value )) FLAGS |= (VALUE1.flags & flags_i16b); \
    if ((VALUE1.flags & flags_i32b) && (VALUE1.uint32_value  <= MEM64->uint32_value  ) && (MEM64->uint32_value  >= VALUE2.uint32_value )) FLAGS |= (VALUE1.flags & flags_i32b); \
    if ((VALUE1.flags & flags_i64b) && (VALUE1.uint64_value  <= MEM64->uint64_value  ) && (MEM64->uint64_value  >= VALUE2.uint64_value )) FLAGS |= (VALUE1.flags & flags_i64b); \
    if ((VALUE1.flags & flag_f32b ) && (VALUE1.float32_value <= MEM64->float32_value ) && (MEM64->float32_value >= VALUE2.float32_value)) FLAGS |= (flag_f32b);                 \
    if ((VALUE1.flags & flag_f64b ) && (VALUE1.float64_value <= MEM64->float64_value ) && (MEM64->float64_value >= VALUE2.float64_value)) FLAGS |= (flag_f64b);

#define FLAG_REMOVE_IF_NOT_IN_RANGE(FLAGS, MEM64, VALUE1, VALUE2)                                                                                                                                            \
    if ((FLAGS & flags_i8b )                                 && !((VALUE1.uint8_value   <= MEM64->uint8_value   ) && (MEM64->uint8_value   >= VALUE2.uint8_value  ))) FLAGS &= ~(VALUE1.flags & flags_i8b);  \
    if ((FLAGS & flags_i16b)                                 && !((VALUE1.uint16_value  <= MEM64->uint16_value  ) && (MEM64->uint16_value  >= VALUE2.uint16_value ))) FLAGS &= ~(VALUE1.flags & flags_i16b); \
    if ((FLAGS & flags_i32b)                                 && !((VALUE1.uint32_value  <= MEM64->uint32_value  ) && (MEM64->uint32_value  >= VALUE2.uint32_value ))) FLAGS &= ~(VALUE1.flags & flags_i32b); \
    if ((FLAGS & flags_i64b)                                 && !((VALUE1.uint64_value  <= MEM64->uint64_value  ) && (MEM64->uint64_value  >= VALUE2.uint64_value ))) FLAGS &= ~(VALUE1.flags & flags_i64b); \
    if ((FLAGS & flag_f32b ) && !isnan(MEM64->float64_value) && !((VALUE1.float32_value <= MEM64->float32_value ) && (MEM64->float32_value >= VALUE2.float32_value))) FLAGS &= ~(flag_f32b);                 \
    if ((FLAGS & flag_f64b ) && !isnan(MEM64->float64_value) && !((VALUE1.float64_value <= MEM64->float64_value ) && (MEM64->float64_value >= VALUE2.float64_value))) FLAGS &= ~(flag_f64b);

bool
Scanner::first_scan(scan_data_type_t data_type, const string &text)
{
    uservalue_t vals[2];
    scan_match_type_t match_type;
    try {
        string_to_uservalue(data_type, text, &match_type, vals);
    } catch (bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        return false;
        //todo в first_scan() вообще не должно быть string параметра. Всё делается на стороне фронт енда.
    }
    
    if (!handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }
    
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : handle->regions) {
        if (region.start > region.end) {
            clog<<"error: invalid region: empty range: "<<region<<endl;
            return false;
        }
        total_scan_bytes += region.end - region.start;
    }
    
    if (total_scan_bytes == 0) {
        if (handle->regions.size() == 0)
            clog<<"error: no regions defined, perhaps you deleted them all?"<<endl;
        else
            clog<<"error: "<<handle->regions.size()<<" regions defined, but each have 0 bytes"<<endl;
        return false;
    }
// Release
//    Searching took 0.00681768 seconds.
//    We currently have 333305 matches.
    
    matches.clear();
    scan_progress = 0.0;
    stop_flag = false;
    static chrono::duration counted_1 = high_resolution_clock::duration::zero();
    static chrono::duration counted_2 = high_resolution_clock::duration::zero();
    high_resolution_clock::time_point t1, t2;


    switch (data_type) {
        case BYTEARRAY:
            
            break;
        case STRING:
            
            break;
        default:
            match_flags flags;
            switch (match_type) {
                case MATCHANY:
                    matches.reserve(total_scan_bytes);
                    for(; i_nextRegion(); ) {
                        for(i_offset = 0;  i_totalsize > 0;  i_offset += step, i_totalsize -= step) {
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);
                            matches.emplace_back(i_region, i_offset, *i_memory, vals[0].flags);
                        }
                    }
                    break;
                case MATCHEQUALTO:
                    for(; i_nextRegion(); ) {
/// Мы же не можем просто взять, и сказать, что последний байт в регионе может быть int'ом
#define SCAN_BY(COMPARATOR)                                                                                             \
                        for(i_offset = 0;  i_totalsize >= 8 && !stop_flag;  i_offset += step, i_totalsize -= step) {    \
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);                                \
                            flags = flags_empty;                                                                        \
                            FLAG_ADD_IF_COMPARED_BY(flags, i_memory, COMPARATOR, vals[0]);                              \
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);                      \
                        }                                                                                               \
                        for(            ;  i_totalsize >= 4 && !stop_flag;  i_offset += step, i_totalsize -= step) {    \
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);                                \
                            flags = vals[0].flags & ~(flags_64b);                                                       \
                            FLAG_REMOVE_IF_NOT_COMPARED_BY(flags, i_memory, COMPARATOR, vals[0]);                       \
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);                      \
                        }                                                                                               \
                        for(            ;  i_totalsize >= 2 && !stop_flag;  i_offset += step, i_totalsize -= step) {    \
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);                                \
                            flags = vals[0].flags & ~(flags_64b | flags_32b);                                           \
                            FLAG_REMOVE_IF_NOT_COMPARED_BY(flags, i_memory, COMPARATOR, vals[0]);                       \
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);                      \
                        }                                                                                               \
                        for(            ;  i_totalsize >  0 && !stop_flag;  i_offset += step, i_totalsize -= step) {    \
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);                                \
                            flags = vals[0].flags & ~(flags_64b | flags_32b | flags_16b);                               \
                            FLAG_REMOVE_IF_NOT_COMPARED_BY(flags, i_memory, COMPARATOR, vals[0]);                       \
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);                      \
                        }
                        SCAN_BY(==);
                    }
                    break;
                case MATCHNOTEQUALTO:
                    for(; i_nextRegion(); ) {
                        SCAN_BY(!=);
                    }
                    break;
                case MATCHGREATERTHAN:
                    for(; i_nextRegion(); ) {
                        SCAN_BY(>);
                    }
                    break;
                case MATCHLESSTHAN:
                    for(; i_nextRegion(); ) {
                        SCAN_BY(<);
                    }
                    break;
                case MATCHRANGE:
                    for(; i_nextRegion(); ) {
                        for(i_offset = 0;  i_totalsize >= 8;  i_offset += step, i_totalsize -= step) {
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);
                            flags = flags_empty;
                            FLAG_ADD_IF_IN_RANGE(flags, i_memory, vals[0], vals[1]);
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);
                        }
                        for(            ;  i_totalsize >= 4;  i_offset += step, i_totalsize -= step) {
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);
                            flags = vals[0].flags & ~(flags_64b);
                            FLAG_REMOVE_IF_NOT_IN_RANGE(flags, i_memory, vals[0], vals[1]);
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);
                        }
                        for(            ;  i_totalsize >= 2;  i_offset += step, i_totalsize -= step) {
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);
                            flags = vals[0].flags & ~(flags_64b | flags_32b);
                            FLAG_REMOVE_IF_NOT_IN_RANGE(flags, i_memory, vals[0], vals[1]);
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);
                        }
                        for(            ;  i_totalsize > 0;  i_offset += step, i_totalsize -= step) {
                            i_memory = reinterpret_cast<mem64_t *>(&i_buffer[i_offset]);
                            flags = vals[0].flags & ~(flags_64b | flags_32b | flags_16b);
                            FLAG_REMOVE_IF_NOT_IN_RANGE(flags, i_memory, vals[0], vals[1]);
                            if (flags) matches.emplace_back(i_region, i_offset, *i_memory, flags);
                        }
                    }
                    break;
                default:
                    clog<<"Error: called first_scan but data_type not first_scan"<<endl;
            }
    }
    
    clog<<"Done: "<<this->matches.size()<<" matches."<<endl;
    clog<<"Method 1: counted: "<<duration_cast<duration<double>>(counted_1).count()<<" seconds."<<endl;
    clog<<"Method 2: counted: "<<duration_cast<duration<double>>(counted_2).count()<<" seconds."<<endl;
    
    this->scan_progress = 1.0;

    return true;
}



bool
Scanner::next_scan(scan_data_type_t data_type, const string &text)
{
    uservalue_t vals[2];
    scan_match_type_t match_type = MATCHEQUALTO;
    try {
        string_to_uservalue(data_type, text, &match_type, vals);
    } catch (bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
    }

    if (!handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }

    if (matches.size() == 0) {
        clog<<"there are currently no matches"<<endl;
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
            vector<match> matches_new;
            matches_new.reserve(matches.size());
            match_flags flags;
            
            /// MATCHUPDATE
            for(int i=0; i<matches.size(); i++) {
                /// read into memory_ptr
                if (!handle->read(i_memory,
                                  matches[i].address.data,
                                  flags_to_memlength(data_type, matches[i].flags)))
                {
                    /// It should not reached until first_scan will handle last 1 byte in region as 2 or more bytes
                    clog<<"error: cant read memory: "<<hex<<matches[i].address.data<<dec<<endl;
                    clog<<"flags_to_memlength(data_type, this->matches[i].flags): "<<flags_to_memlength(data_type, matches[i].flags)<<endl;
                    clog<<"handle->getRegionOfAddress(this->matches[i].address.data)->end: "<<hex<<handle->getRegionOfAddress(matches[i].address.data)->end<<dec<<endl;
                    clog<<endl;
                }
                
                matches[i].val_old = matches[i].val;
                matches[i].val = *i_memory;
            }
            
            switch (match_type) {
                case MATCHEQUALTO:
                    for(int i=0; i<matches.size(); i++) {
                        flags = flags_empty;
                        if ((matches[i].flags & flag_s64b) && i_memory->int64_value   == vals[0].int64_value)   flags |= flag_s64b;
                        if ((matches[i].flags & flag_s32b) && i_memory->int32_value   == vals[0].int32_value)   flags |= flag_s32b;
                        if ((matches[i].flags & flag_s16b) && i_memory->int16_value   == vals[0].int16_value)   flags |= flag_s16b;
                        if ((matches[i].flags & flag_s8b)  && i_memory->int8_value    == vals[0].int8_value)    flags |= flag_s8b;
                        if ((matches[i].flags & flag_u64b) && i_memory->uint64_value  == vals[0].uint64_value)  flags |= flag_u64b;
                        if ((matches[i].flags & flag_u32b) && i_memory->uint32_value  == vals[0].uint32_value)  flags |= flag_u32b;
                        if ((matches[i].flags & flag_u16b) && i_memory->uint16_value  == vals[0].uint16_value)  flags |= flag_u16b;
                        if ((matches[i].flags & flag_u8b)  && i_memory->uint8_value   == vals[0].uint8_value)   flags |= flag_u8b;
                        if ((matches[i].flags & flag_f64b) && i_memory->float64_value == vals[0].float64_value) flags |= flag_f64b;
                        if ((matches[i].flags & flag_f32b) && i_memory->float32_value == vals[0].float32_value) flags |= flag_f32b;
                        if (flags) {
                            matches[i].flags &= flags;
                            matches_new.emplace_back(matches[i]);
                        }
                        if (this->stop_flag) return false;
                    }
                    this->matches = matches_new;
                    break;
                case MATCHNOTEQUALTO:break;
                case MATCHGREATERTHAN:break;
                case MATCHLESSTHAN:break;
                case MATCHRANGE:break;
                case MATCHNOTCHANGED:break;
                case MATCHCHANGED:break;
                case MATCHINCREASED:break;
                case MATCHDECREASED:break;
                case MATCHINCREASEDBY:break;
                case MATCHDECREASEDBY:break;
            }
    }
    
    clog<<"Done: "<<this->matches.size()<<" matches, in: "<<(duration_cast<duration<double>>(high_resolution_clock::now() - time_point)).count()<<" second."<<endl;
    
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
