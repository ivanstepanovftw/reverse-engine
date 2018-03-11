//
// Created by root on 09.03.18.
//

#include <bitset>
#include "scanner.hh"
#include "value.hh"

using namespace std;

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
Scanner::parse(scan_data_type_t data_type, const char *ustr, scan_match_type_t *mt, uservalue_t *vals) {
    switch (data_type) {
        case BYTEARRAY:
//            if (!parse_uservalue_bytearray(ustr, &vals[0])) {
//                printf("AOB parse error\n");
//                return false;
//            }
            printf("AOB parse not implemented yet\n");
            return false;
        case STRING:
            printf("String parse not implemented yet\n");
            return false;
        default:
            /// detect a range
            char *pos = strstr(const_cast<char *>(ustr), "..");
            if (pos) {
                *pos = '\0';
                if (!parse_uservalue_default(ustr, &vals[0]))
                    return false;
                ustr = pos + 2;
                if (!parse_uservalue_default(ustr, &vals[1]))
                    return false;
                
                /* Check that the range is nonempty */
                if (vals[0].float64_value > vals[1].float64_value) {
                    printf("Empty range\n");
                    return false;
                }
                
                /* Store the bitwise AND of both flags in the first value,
                 * so that range scanroutines need only one flag testing. */
                vals[0].flags = static_cast<match_flags>(vals[0].flags&vals[1].flags);
                *mt = MATCHRANGE;
            } else {
                *mt = MATCHEQUALTO;
                if (!parse_uservalue_default(ustr, &vals[0]))
                    return false;
            }
            break;
    }
    
    vals[0].flags = static_cast<match_flags>(vals[0].flags&scan_data_type_to_flags[data_type]);
    vals[1].flags = static_cast<match_flags>(vals[1].flags&scan_data_type_to_flags[data_type]);
    return true;
}


bool
Scanner::first_scan(scan_data_type_t data_type, const char *ustr)
{
    uservalue_t vals[2];
    scan_match_type_t mt;
    if (!this->parse(data_type, ustr, &mt, vals)) {
        return false;
    }
    
    if (!this->handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }
    
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : this->handle->regions) {
        if (region.start > region.end) {
            clog<<"error: invalid region. filename: \""<<region.filename<<"\", start: "<<region.start<<" end: "<<region.end<<endl;
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
    
    this->matches.clear();
    this->scan_progress = 0.0;
    this->stop_flag = false;
    
    /// for each region scan for a uservalue
    for(const region_t &region : this->handle->regions) {
        if (!region.writable || !region.readable)
            continue;
//        /// we will use chunks for better memory usage
//        uint8_t buffer[CHUNK_SIZE];
//        uintptr_t totalsize = region.end - region.start;
//        uintptr_t chunknum = 0;
//        while (totalsize) {
//            uintptr_t readsize = (totalsize < CHUNK_SIZE) ? totalsize : CHUNK_SIZE;
//            uintptr_t readaddr = region.start + CHUNK_SIZE * chunknum;
//            bzero(buffer, CHUNK_SIZE); //should we use it?
//            
//            /// read into buffer
//            if (this->handle->read(buffer, (void *) readaddr, readsize)) {
//                for(uintptr_t cursor = 0; cursor < readsize; cursor += 1) {
//                    // etc ...
//                }
//            }
//            
//            // memory       |_|_|_|_|_|_|_|x|x|_|_|_|_|_|_|_|_|_|_|_|
//            // buf 8 bytes  [               ] <- lost 1 byte for our int16_t value
//            totalsize -= MAX(readsize-7, totalsize);
//            chunknum++;
//        }
        uintptr_t totalsize = region.end - region.start;
        uint8_t buffer[totalsize];
        
        /// read into buffer
        if (!this->handle->read(buffer, (void *) region.start, totalsize)) {
            clog<<"error: cant read memory region. filename: \""<<region.filename<<"\", start: "<<region.start<<" end: "<<region.end<<endl;
            break;
        }
        
        for(uintptr_t offset = 0; totalsize > 0; totalsize-=this->step, offset+=this->step) {
            const mem64_t *memory_ptr = reinterpret_cast<mem64_t *>(&buffer[offset]);
            
            switch (mt) {
                case MATCHEQUALTO: //fixme не удаляет флаг
                    if ((vals[0].flags & flag_s64b) && memory_ptr->int64_value  == vals[0].int64_value
                    ||  (vals[0].flags & flag_s32b) && memory_ptr->int32_value  == vals[0].int32_value
                    ||  (vals[0].flags & flag_s16b) && memory_ptr->int16_value  == vals[0].int16_value
                    ||  (vals[0].flags & flag_s8b ) && memory_ptr->int8_value   == vals[0].int8_value
                    ||  (vals[0].flags & flag_u64b) && memory_ptr->uint64_value == vals[0].uint64_value
                    ||  (vals[0].flags & flag_u32b) && memory_ptr->uint32_value == vals[0].uint32_value
                    ||  (vals[0].flags & flag_u16b) && memory_ptr->uint16_value == vals[0].uint16_value
                    ||  (vals[0].flags & flag_u8b ) && memory_ptr->uint8_value  == vals[0].uint8_value
                            ) {
                        clog<<"address: "<<hex<<(region.start+offset)<<dec<<", flags: "<<bitset<16>(vals[0].flags)<<endl;
                        this->matches.emplace_back(&region, offset, *memory_ptr, vals[0].flags);
                    }
                    break;
                case MATCHNOTEQUALTO:
                    break;
                case MATCHGREATERTHAN:break;
                case MATCHLESSTHAN:break;
                case MATCHRANGE:
                    break;
                case MATCHINCREASEDBY:break;
                case MATCHDECREASEDBY:break;
                case MATCHANY:break;
                case MATCHUPDATE:break;
                case MATCHNOTCHANGED:break;
                case MATCHCHANGED:break;
                case MATCHINCREASED:break;
                case MATCHDECREASED:break;
            }
    
            /// stop scanning if asked to
            if (this->stop_flag)
                break;
        }
        
        /// stop scanning if asked to
        if (this->stop_flag)
            break;
    }
    
    this->scan_progress = 1.0;
    printf("we currently have %ld matches.\n", this->matches.size());
    return true;
}


//bool
//Scanner::next_scan(scan_data_type_t data_type, char *ustr)
//{
//    uservalue_t vals[2];
//    scan_match_type_t m = MATCHEQUALTO;
//    
//    if (!parse(data_type, ustr, &m, vals)) {
//        return false;
//    }
//    
//    if (!handle->isRunning()) {
//        return false;
//    }
//    
//    if (matches.size() == 0) {
//        printf("there are currently no matches.\n");
//        return false;
//    }
//    /* already know some matches */
//    if (!sm_checkmatches(m, &vals[0])) {
//        printf("failed to search target address space.\n");
//        return false;
//    }
//}


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
