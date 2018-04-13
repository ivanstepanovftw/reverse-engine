//
// Created by root on 09.03.18.
//

#include <bitset>
#include <chrono>
#include <cmath>
#include <fcntl.h>
#include "scanner.hh"
#include "value.hh"
#include <fcntl.h>
#include <sys/mman.h>

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


void
Scanner::string_to_uservalue(const scan_data_type_t &data_type, const string &text,
                             scan_match_type_t *match_type, uservalue_t *uservalue)
{
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


char *
Scanner::snapshot(int fdin) {
    /// Allocate space
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : handle->regions)
        total_scan_bytes += region.end - region.start;
    
    if (total_scan_bytes == 0) {
        if (handle->regions.size() == 0)
            clog<<"error: no regions defined, perhaps you deleted them all?"<<endl;
        else
            clog<<"error: "<<handle->regions_all.size()<<" regions defined, but each have 0 bytes"<<endl;
        return nullptr;
    }
    
    if (lseek(fdin, total_scan_bytes, SEEK_SET) == -1)
        perror("error: cant increase fdin size");
    if (write(fdin, "", 1) != 1)
        perror("error: cant increase fdin size 2");
    if (lseek(fdin, 0, SEEK_SET) == -1)
        perror("error: lseek: fdin");
    
    
    /// Create mmap
    char *dst = static_cast<char *>(mmap(nullptr, total_scan_bytes, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_POPULATE, fdin, 0));
    if (dst == MAP_FAILED) {
        perror("error: mmap fdin");
        return nullptr;
    }
    
    
    /// Snapshot goes here
    uintptr_t region_size = 0;
    uintptr_t cursor_dst = 0;
    
    high_resolution_clock::time_point timestamp = high_resolution_clock::now();
    
    for(region_t &region : handle->regions) {
        region_size = region.end - region.start;
        
        memcpy(dst+cursor_dst, &region.start, sizeof(region.start)+sizeof(region.end));
        cursor_dst += sizeof(region.start)+sizeof(region.end);
        
        if (!handle->read(dst+cursor_dst, region.start, region_size)) {
            clog<<"warning: region not copied: region: "<<region<<endl;
            cursor_dst -= sizeof(region.start)+sizeof(region.end);
            total_scan_bytes -= region_size;
            
            if (!handle->isRunning()) {
                clog<<"error: process not running"<<endl;
                return nullptr;
            }
        } else
            cursor_dst += region_size;
    }
    
    clog<<"Done "<<total_scan_bytes<<" bytes, in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    clog<<"Snapshot size: "<<cursor_dst<<" bytes."<<endl;
    if (ftruncate(fdin, cursor_dst) != 0)
        perror("warning: can't ftruncate fdin");
    
    return dst;
}

bool
Scanner::first_scan(const scan_data_type_t data_type, uservalue_t *uservalue, const scan_match_type_t match_type)
{
    if (!handle->isRunning()) {
        clog<<"error: process not running"<<endl;
        return false;
    }
    
    scan_progress = 0.0;
    stop_flag = false;
    
    
    // todo refresh size of stored regions using handle->refreshRegions() method
    handle->updateRegions();
    
    /// Shapshot process memory
    int fdin = open("MEMORY.TMP", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fdin < 0) {
        perror("error: can't open 'MEMORY.TMP'");
        return false;
    }
    
    char *src = snapshot(fdin);
    if (!src)
        return false;
    
    struct stat statbuf;
    if (fstat(fdin, &statbuf) < 0) {
        perror("error: can't fstat fdin");
        return false;
    }
    clog<<"Snapshot size with fstat: "<<statbuf.st_size<<" bytes."<<endl;
    
    
    /// Create file for storing matches
    int fdout = open("ADDRESSES.FIRST", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (fdout < 0) {
        perror("error: can't open 'ADDRESSES.FIRST' for writing");
        return false;
    }
    
    
    /// Allocate space to mmap fdout once
    if (lseek(fdout, statbuf.st_size*sizeof(match) - 1, SEEK_SET) == -1)
        perror("error: cant increase fdout size");
    if (write(fdout, "", 1) != 1)
        perror("error: cant increase fdout size 2");
    
    match *dst = static_cast<match *>(mmap(nullptr, statbuf.st_size*sizeof(match), PROT_WRITE|PROT_READ, MAP_SHARED|MAP_POPULATE, fdout, 0));
    if (dst == MAP_FAILED) {
        perror("error: dst == MAP_FAILED");
        close(fdin);
        close(fdout);
        return false;
    }
    
    
    /// Scanning routines begins here
    region_t region;
    mem64_t mem;
    match m;
    uintptr_t cursor_src = 0;
    uintptr_t cursor_dst = 0;
    
    high_resolution_clock::time_point timestamp = high_resolution_clock::now();
    
    /// Okay, we are scanning only for uint64_t, step: 1
//#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
    
    //* <uglycode> */
#define PART1(FLAGS_CHECK_CODE)\
    cursor_src = 0;\
    cursor_dst = 0;\
    while(cursor_src < statbuf.st_size) {\
        memcpy(&region.start, src+cursor_src, sizeof(region.start)+sizeof(region.end));\
        cursor_src += sizeof(region.start) + sizeof(region.end);\
        /*clog<<"reading region: "<<region<<'\n';*/\
        m.address = region.start;\
        for( ; m.address + 8 <= region.end; cursor_src += step, m.address += step) {\
            m.memory = *reinterpret_cast<mem64_t *>(src+cursor_src);\
            FLAGS_CHECK_CODE\
            if (m.flags)\
                dst[cursor_dst++] = m;\
        }\
        for( ; m.address + 4 <= region.end; cursor_src += step, m.address += step) {\
            m.memory = *reinterpret_cast<mem64_t *>(src+cursor_src);\
            FLAGS_CHECK_CODE\
            m.flags &= ~(flags_64b);\
            if (m.flags)\
                dst[cursor_dst++] = m;\
        }\
        for( ; m.address + 2 <= region.end; cursor_src += step, m.address += step) {\
            m.memory = *reinterpret_cast<mem64_t *>(src+cursor_src);\
            FLAGS_CHECK_CODE\
            m.flags &= ~(flags_64b | flags_32b);\
            if (m.flags)\
                dst[cursor_dst++] = m;\
        }\
        for( ; m.address + 1 <= region.end; cursor_src += step, m.address += step) {\
            m.memory = *reinterpret_cast<mem64_t *>(src+cursor_src);\
            FLAGS_CHECK_CODE\
            m.flags &= ~(flags_64b | flags_32b | flags_16b);\
            if (m.flags)\
                dst[cursor_dst++] = m;\
        }\
    }\
    //* </uglycode> */
    
/* -Ofast
Old:
Done 333112 matches, in: 0.0350032 seconds.     с буфером
Done 333112 matches, in: 0.366467 seconds.      без буфера, с инлайном
Done 333110 matches, in: 0.0190443 seconds.     свой буфер, с инлайном
Done 333208 matches, in: 0.0213902 seconds.     всё в буферах
Done 356352 bytes, in: 0.000642586 seconds.     буфера+диск
Done 333154 matches, in: 0.0526076 seconds.
Done 1352589312 bytes, in: 4.51307 seconds.     ищем едичикку int 64 в доте
Done 738743 matches, in: 11.3695 seconds.

Using buffered reader & writer (fakemem 428 MiB):
Done 449150976 bytes, in: 2.07514 seconds.
Done 449128277 matches, in: 81.3764 seconds.
Done 449128277 matches, in: 69.7533 seconds.
Done 449128277 matches, in: 75.5361 seconds.

v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ !! YOU ARE HERE !!
Using mmap
 (fakemem 111 MiB):
Done 116752384 bytes, in: 0.125883 seconds.
Done 116716123 matches, in: 13.3575 seconds.
Done 116598353 matches, in: 16.7437 seconds.    CPU.throttling.enable()
Done 116598353 matches, in: 15.5291 seconds.
Done 116598353 matches, in: 13.012 seconds.     CPU.throttling.disable()
Done 116598353 matches, in: 13.2313 seconds.    yeah that was throttling

 (fakemem 33 MiB):
Done 34797174 matches, in: 2.35997 seconds. (debug)
Done 34797174 matches, in: 2.19174 seconds.
*/
    
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
    
    clog<<"Done "<<cursor_dst<<" matches, in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    // free the mmapped memory
    if (munmap(src, static_cast<size_t>(statbuf.st_size)) == -1) {
        perror("error: munmap src");
        close(fdin);
        close(fdout);
        return false;
    }
    close(fdin);
    if (munmap(dst, cursor_dst+1) == -1) {
        perror("error: munmap dst");
        close(fdout);
        return false;
    }
    close(fdout);
    
    
    /// В планах оставить сканирование без mmaping'а для самых счастливых
    /*
    uintptr_t b;
    vector<match> mm;
    
    
    timestamp = high_resolution_clock::now();
    for(region_t &region : handle->regions) {
        region_size = region.end - region.start;
        char buffer[region_size];
        if (!handle->read(buffer, region.start, region_size)) {
            clog<<"error: invalid region: cant read memory: "<<region<<endl;
            return false;
        }
        
        for(b = 0;         region_size  >= 8;  b += step, region_size  -= step) {
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
        for(            ;  region_size  >= 4;  b += step, region_size  -= step) {
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
        for(            ;  region_size  >= 2;  b += step, region_size  -= step) {
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
        for(            ;  region_size  >= 1;  b += step, region_size  -= step) {
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
