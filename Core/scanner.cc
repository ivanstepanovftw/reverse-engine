//
// Created by root on 09.03.18.
//

#include <bitset>
#include <chrono>
#include <cmath>
#include "scanner.hh"
#include "value.hh"

#undef	MAX
#define MAX(a, b)  (((a) >= (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define HEX(s) hex<<showbase<<(s)<<dec

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


bool
Scanner::snapshot() {
    if (!handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }
    
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : handle->regions)
        total_scan_bytes += region.end - region.start;
    
    if (total_scan_bytes == 0) {
        if (handle->regions.size() == 0)
            clog<<"error: no regions defined, perhaps you deleted them all?"<<endl;
        else
            clog<<"error: "<<handle->regions_all.size()<<" regions defined, but each have 0 bytes"<<endl;
        return false;
    }
    
    fsnapshot.open("MEMORY.TMP", ios::in | ios::out | ios::binary | ios::trunc);
    const size_t chunksize = 0x0F'00'00'00/match::size();  // 240 MiB
    char *buffer = new char[chunksize];
    uintptr_t totalsize;
    uintptr_t readsize;
    uintptr_t readaddr;
    uintptr_t chunknum;
    
    high_resolution_clock::time_point timestamp = high_resolution_clock::now();
    
    fsnapshot.seekp(0, ios::end);
    for(region_t &region : handle->regions) {
        totalsize = region.end - region.start;
        fsnapshot.write(reinterpret_cast<const char *>(&region.start), 2*sizeof(region.start));
        chunknum = 0;
        while(totalsize > 0) {
            readsize = MIN(totalsize, chunksize);
            readaddr = region.start + (chunksize * chunknum);
            bzero(buffer, chunksize);
            if (!handle->read(buffer, readaddr, readsize))
                if (!handle->isRunning()) {
                    clog<<"error: process not running"<<endl;
                    return false;
                }
            fsnapshot.write(buffer, readsize);
            totalsize -= readsize;
            chunknum++;
        }
    }
    delete[] buffer;
    
    clog<<"Done "<<total_scan_bytes<<" bytes, in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    return true;
}

bool
Scanner::first_scan(const scan_data_type_t data_type, uservalue_t *uservalue, const scan_match_type_t match_type)
{
    matches.clear();
    scan_progress = 0.0;
    stop_flag = false;
    
    if (!snapshot())
        return false;
    
    if (!fsnapshot)
        clog<<"error: fsnapshot"<<endl;
    if (fsnapshot.good())
        clog<<"good..."<<endl;
    
    
    
// 
// Плюсы данного метода: +++++++++++++++++++++++++++++
// Минусы: 
// Вывод: круто
//
// У нас есть файл размером 61 байт
// [ .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ]
// 
// Пусть первые два байта говорят о начале и конце региона, следовательно, говорят о размере этого региона
// Давай разобьём наш файл на регионы
// [ 04 0F[.. .. .. .. .. .. .. .. .. .. ..]20 4E[.. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..]]
//   ^~~~~ размер региона 11 байт           ^~~~~ размер региона 46 байт     10                            20                            30                            40                46             
// 
// У нас размер буфера 20 байт
// Рассмотриваем первый регион
// Его размер не больше размера чанка (11 <= 20), следовательно, СКАНИРУЕМ наш буфер, соблюдая флаги в конце региона
// 
// Терерь рассматриваем второй регион, его размер больше размеру одного чанка
// Размер чанка 20 байт
// [01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14][15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28][29 30 31 32 33 34]
// 
// 
// 
// Читаем в буффер ПЕРВЫЙ чанк
// [00 00 00 00 00 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14]
//                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//     Теперь СКАНИРУЕМ наш буфер, пропуская первые 7 и последние 7, что бы не добавлял лишнего
//     [00 00 00 00 00 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14]
//                           ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     
//         Пропущеные байты переносим в начало
//         [0E 0F 10 11 12 13 14 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14]
//          ^~~~~~~~~~~~~~~~~~~~
//
// Читаем в буфер ВТОРОЙ чанк
// [0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28]
//                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//     Теперь СКАНИРУЕМ наш буфер, пропуская последние 7, что бы не добавлял лишнего
//     [0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28]
//      ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     
//         Пропущеные байты переносим в начало
//         [22 23 24 25 26 27 28 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28]
//          ^~~~~~~~~~~~~~~~~~~~
// 
// Читаем в буфер ТРЕТИЙ чанк, неполноценный
// [22 23 24 25 26 27 28 29 30 31 32 33 34 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28]
//                       ^~~~~~~~~~~~~~~~~
//     
//     Теперь СКАНИРУЕМ наш буфер, соблюдая флаги в конце региона
//     [22 23 24 25 26 27 28 29 30 31 32 33 34 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28]
//      ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//     
//
    
    
    /// You can remove comment below if you want to debug PART1 macro. Use CLion middle-mouse button to remove all "\".
#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
    
    
    streamoff total_size;
    region_t region;
    const size_t CHUNK_SIZE = 0x0F'00'00'00/match::size();  // 240 MiB
    const size_t RESERVED = sizeof(mem64_t) - 1;            // 7 elements reserved from start
    const size_t BUFFER_SIZE = CHUNK_SIZE + RESERVED;
    
    char *buffer = new char[BUFFER_SIZE];
    char *chunk = buffer + RESERVED;
    bzero(buffer, BUFFER_SIZE);
    
    uintptr_t region_remain_size;
    uintptr_t offset;
    uintptr_t off;
    uintptr_t chunk_count;
    uintptr_t lshift;
    uintptr_t rshift;
    uintptr_t read_address;
    uintptr_t matches_count = 0;
    
    
#define PART1(FLAGS_CHECK_CODE)
    /* определяем размер файла */
    fsnapshot.seekg(0, ios::end);
    total_size = fsnapshot.tellg();
    fsnapshot.seekg(0, ios::beg);
    
    /* для каждого региона */
    while(fsnapshot.tellg() < total_size) {
        /* читаем его размер */
        fsnapshot.read(reinterpret_cast<char *>(&region.start), 2*sizeof(region.start));
        region_remain_size = region.end - region.start;
        clog<<"reading region: "<<region<<'\n';
        
        chunk_count = 0;
        offset = RESERVED;
        
        /* если придётся читать по чанкам */
        if (region_remain_size > CHUNK_SIZE) {
            clog<<"reading chunk-by-chunk. chunk_size: "<<CHUNK_SIZE<<'\n';
            while(region_remain_size > CHUNK_SIZE) {
                /* Читаем в буффер один чанк */
                fsnapshot.read(chunk, CHUNK_SIZE);
                
                /* Теперь сканируем буффер */
                read_address = region.start + CHUNK_SIZE * chunk_count;
                for(off = 0; offset < CHUNK_SIZE - RESERVED; off++, offset++) {
                    match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
                    FLAGS_CHECK_CODE
                    if (m.flags) {
                        matches_count += 1;
                        matches.append(m);
                    }
                }
                /* todo я спать 
                for(off = 0;  region_remain_size >= 8;  off += step, offset += step, region_remain_size -= step) {
                    match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
                    FLAGS_CHECK_CODE
                    if (m.flags) {
                        matches_count += 1;
                        matches.append(m);
                    }
                }*/
                
                /* Пропущеные байты переносим в начало */
                for(lshift = 0, rshift = CHUNK_SIZE; lshift < RESERVED; lshift++, rshift++)
                    buffer[lshift] = buffer[rshift];
                
                offset = 0;
                region_remain_size -= CHUNK_SIZE;
                chunk_count++;
            }
            clog<<"done "<<chunk_count<<" chunks amd "<<matches_count<<" matches added"<<'\n';
        }
        offset = RESERVED;
        
        /* ниже уже либо маленький регион для буфера, либо последний чанк */
        /* Читаем в буффер один чанк */
        bzero(chunk+region_remain_size, 7);
        fsnapshot.read(chunk, region_remain_size);
        clog<<"reading "<<region_remain_size<<" bytes"<<'\n';
        
        /* Теперь сканируем буффер */
        read_address = region.start + CHUNK_SIZE * chunk_count;
        
        for(off = 0;  region_remain_size >= 8;  off += step, offset += step, region_remain_size -= step) {
            match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
            FLAGS_CHECK_CODE
            if (m.flags)
                matches.append(m);
        }
        for(       ;  region_remain_size >= 4;  off += step, offset += step, region_remain_size -= step) {
            match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b);
            if (m.flags)
                matches.append(m);
        }
        for(       ;  region_remain_size >= 2;  off += step, offset += step, region_remain_size -= step) {
            match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b);
            if (m.flags)
                matches.append(m);
        }
        for(       ;  region_remain_size >= 1;  off += step, offset += step, region_remain_size -= step) {
            match m(read_address + off, *reinterpret_cast<mem64_t *>(&buffer[offset]));
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b | flags_16b);
            if (m.flags)
                matches.append(m);
        }
    }
    matches.flush();
//end PART1
    
// Это всё под CMake Optimized
//    Done 333112 matches, in: 0.0350032 seconds. с буфером
//    Done 333112 matches, in: 0.366467 seconds. без буфера, с инлайном
//    Done 333110 matches, in: 0.0190443 seconds. свой буфер, с инлайном
//    Done 333208 matches, in: 0.0213902 seconds. всё в буферах
//    Done 356352 bytes, in: 0.000642586 seconds. буфера+диск
//    Done 333154 matches, in: 0.0526076 seconds.
//    Done 1352589312 bytes, in: 4.51307 seconds. ищем едичикку int 64 в доте
//    Done 738743 matches, in: 11.3695 seconds.
//    ss CHUNK_SIZE                : 93b13b
//    ss CHUNK_SIZE+sizeof(mem64_t): 93b143
//    Done 1366437888 bytes, in: 6.96247 seconds.
//            good...
//CHUNK_SIZE                : 93b13b
//    CHUNK_SIZE+sizeof(mem64_t): 93b143
//    Done 930064926 matches, in: 179.071 seconds.
    
    high_resolution_clock::time_point timestamp = high_resolution_clock::now();
    if (data_type == BYTEARRAY) {
        clog<<"not supported"<<endl;
    }
    else if (data_type == STRING) {
        clog<<"not supported"<<endl;
    } else {
        switch(match_type) {
                case MATCHANY:
                    PART1(
                            m.flags = flags_all;
                    )
                    break;
                case MATCHEQUALTO:
                    PART1(
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
                    PART1(
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
                    PART1(
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
                    PART1(
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
                    PART1(
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
                    clog<<"error: only first_scan supported"<<endl;
            }
    }
    delete[] buffer;
    
    clog<<"Done "<<matches.size()<<" matches, in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    /// Проверка
    /*
    uintptr_t b;
    vector<match> mm;
    
    
    timestamp = high_resolution_clock::now();
    for(region_t &region : handle->regions) {
        region_remain_size = region.end - region.start;
        char buffer[region_remain_size];
        if (!handle->read(buffer, region.start, region_remain_size)) {
            clog<<"error: invalid region: cant read memory: "<<region<<endl;
            return false;
        }
        
        for(b = 0;         region_remain_size  >= 8;  b += step, region_remain_size  -= step) {
            match m(region.start + b, *reinterpret_cast<mem64_t *>(&buffer[b]));
            m.flags = flags_empty;
            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
            if (m.flags) mm.push_back(m);
        }
        for(            ;  region_remain_size  >= 4;  b += step, region_remain_size  -= step) {
            match m(region.start + b, *reinterpret_cast<mem64_t *>(&buffer[b]));
            m.flags = flags_empty;
            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
            m.flags &= ~(flags_64b);\
            if (m.flags) mm.push_back(m);
        }
        for(            ;  region_remain_size  >= 2;  b += step, region_remain_size  -= step) {
            match m(region.start + b, *reinterpret_cast<mem64_t *>(&buffer[b]));
            m.flags = flags_empty;
            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
            m.flags &= ~(flags_64b | flags_32b);
            if (m.flags) mm.push_back(m);
        }
        for(            ;  region_remain_size  >= 1;  b += step, region_remain_size  -= step) {
            match m(region.start + b, *reinterpret_cast<mem64_t *>(&buffer[b]));
            m.flags = flags_empty;
            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
            m.flags &= ~(flags_64b | flags_32b | flags_16b);
            if (m.flags) mm.push_back(m);
        }
    }
    
    clog<<"Done "<<mm.size()<<" matches, in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    vector<match> notfound;
    bool found;
    
    size_t i1, i2;
    
    for(i1 = 0, i2 = 0; i1 < mm.size(); i1++) {
        match m1 = mm[i1];
        match m2 = matches.get(i2);
        
        if (m1.address != m2.address) {
            region_t *r1 = handle->getRegionOfAddress(m1.address);
            region_t *r2 = handle->getRegionOfAddress(m2.address);
            clog<<"i1: "<<i1<<", .start: "<<HEX(r1->start)<<", .end: "<<HEX(r1->end)<<", address: "<<HEX(m1.address)<<'\n';
            clog<<"i2: "<<i2<<", .start: "<<HEX(r2->start)<<", .end: "<<HEX(r2->end)<<", address: "<<HEX(m2.address)<<'\n';
        } else
            i2++;
    }
    */
    
    clog<<"Done."<<endl;
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

//    if (matches.size() == 0) {
//        clog<<"there are currently no matches"<<endl;
//        return false;
//    }
    
    this->scan_progress = 0.0;
    this->stop_flag = false;
//    auto time_point = high_resolution_clock::now();
    
//    mem64_t i_memory;
    
    if (data_type == BYTEARRAY) {
        clog<<"not supported"<<endl;
    }
    else if (data_type == STRING) {
        clog<<"not supported"<<endl;
    } else {
//        match_flags flags;
//        
//        /// MATCHUPDATE
//        for(int i=0; i<matches.size(); i++) {
//            /// read into memory_ptr
//            if (!handle->read(&i_memory,
//                              matches[i].address.data,
//                              flags_to_memlength(data_type, matches[i].flags)))
//            {
//                /// It should not reached until first_scan will handle last 1 byte in region as 2 or more bytes
//                clog<<"error: cant read memory: "<<hex<<matches[i].address.data<<dec<<endl;
//                clog<<"flags_to_memlength(data_type, this->matches[i].flags): "<<flags_to_memlength(data_type, matches[i].flags)<<endl;
//                clog<<"handle->getRegionOfAddress(this->matches[i].address.data)->end: "<<hex<<handle->getRegionOfAddress(matches[i].address.data)->end<<dec<<endl;
//                clog<<endl;
//            }
//            
//            matches[i].mem_old = matches[i].mem;
//            matches[i].mem = i_memory;
//        }
        
        switch (match_type) {
            case MATCHEQUALTO:
//                for(int i=0; i<matches.size(); i++) {
//                    flags = flags_empty;
//                    if ((matches[i].flags & flag_s64b) && i_memory.int64_value   == vals[0].int64_value)   flags |= flag_s64b;
//                    if ((matches[i].flags & flag_s32b) && i_memory.int32_value   == vals[0].int32_value)   flags |= flag_s32b;
//                    if ((matches[i].flags & flag_s16b) && i_memory.int16_value   == vals[0].int16_value)   flags |= flag_s16b;
//                    if ((matches[i].flags & flag_s8b)  && i_memory.int8_value    == vals[0].int8_value)    flags |= flag_s8b;
//                    if ((matches[i].flags & flag_u64b) && i_memory.uint64_value  == vals[0].uint64_value)  flags |= flag_u64b;
//                    if ((matches[i].flags & flag_u32b) && i_memory.uint32_value  == vals[0].uint32_value)  flags |= flag_u32b;
//                    if ((matches[i].flags & flag_u16b) && i_memory.uint16_value  == vals[0].uint16_value)  flags |= flag_u16b;
//                    if ((matches[i].flags & flag_u8b)  && i_memory.uint8_value   == vals[0].uint8_value)   flags |= flag_u8b;
//                    if ((matches[i].flags & flag_f64b) && i_memory.float64_value == vals[0].float64_value) flags |= flag_f64b;
//                    if ((matches[i].flags & flag_f32b) && i_memory.float32_value == vals[0].float32_value) flags |= flag_f32b;
//                    if (flags) {
//                        matches[i].flags &= flags;
//                        matches_new.emplace_back(matches[i]);
//                    }
//                    if (this->stop_flag) return false;
//                }
//                this->matches = matches_new;
//                break;
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
            default:
                clog<<"error: only next_scan supported"<<endl;
        }
    }
    
//    clog<<"Done: "<<this->matches.size()<<" matches, in: "<<(duration_cast<duration<double>>(high_resolution_clock::now() - time_point)).count()<<" second."<<endl;
    
    this->scan_progress = 1.0;
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
