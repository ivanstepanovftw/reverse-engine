//
// Created by root on 12.02.18.
//

#include <iostream>
#include <iomanip>
#include <chrono>
#include <type_traits>
#include <zconf.h>
#include <functional>
#include <Core/core.hh>
#include <Core/value.hh>
#include <Core/scanner.hh>
#include <cmath>
#include <cassert>

using namespace std;
using namespace std::chrono;
using namespace std::literals;

const string target = "HackMe";

#define HEX(s) hex<<showbase<<(s)<<dec

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int
main() {
    // Todo performance match_flags or uint16_t
    // Todo докажи, что reinterpret_cast<mem64_t *>(&buffer[r_off]) при buffer[1] = {0}; будет представлять ui64 как 0
    /*
     * -funswitch-loops:
    Method A: done 333301 matches, in: 0.0136228 seconds, overall: 1.39055 seconds.
    Method B: done 333301 matches, in: 0.00767857 seconds, overall: 0.618613 seconds.
     * -O0 -funswitch-loops:
    Method A: done 333301 matches, in: 0.0161664 seconds, overall: 1.6087 seconds.
    Method B: done 333301 matches, in: 0.0114342 seconds, overall: 0.975553 seconds.
     * -O0 -fno-unswitch-loops:
    Method A: done 333301 matches, in: 0.0155338 seconds, overall: 1.60998 seconds.
    Method B: done 333301 matches, in: 0.0100367 seconds, overall: 0.958184 seconds.
     * -O3 -funswitch-loops
    Method A: done 333301 matches, in: 0.0326429 seconds, overall: 3.25194 seconds.
    Method B: done 333301 matches, in: 0.0117518 seconds, overall: 1.28925 seconds.
     * -O3 -fno-unswitch-loops
    Method A: done 333301 matches, in: 0.0330074 seconds, overall: 2.85701 seconds.
    Method B: done 333301 matches, in: 0.0126842 seconds, overall: 1.1504 seconds.
     * -O3 -Os -funswitch-loops
    Method A: done 333261 matches, in: 0.0329324 seconds, overall: 2.1788 seconds.
    Method B: done 333261 matches, in: 0.0134741 seconds, overall: 1.10469 seconds.
     *
     * My PC are too slow, CLion takes a lot of processor time, so this shit overheats, drop MHz and results are various.
     * Conclusion: -funswitch-loops doesn't work. Manual unswitching works well.
     */
    /// Trainer and scanner example
    Handle *h = nullptr;
    region_t *exe = nullptr;
    region_t *libc = nullptr;
    region_t *ld = nullptr;
    
stage_waiting:;
    cout<<"Waiting for ["<<target<<"] process"<<endl;
    for(;;) {
        delete h;
        h = new Handle(target);
        if (h->isRunning())
            break;
        usleep(500'000);
    }
    cout<<"Found! PID is ["<<h->pid<<"]"<<endl;
    
stage_updating:;
    for(;;) {
        h->updateRegions();
        exe = h->getRegion();
        libc = h->getRegion("libc-2.26.so");
        ld = h->getRegion("ld-2.26.so");
        if (exe && libc && ld)
            break;
        usleep(500'000);
    }
    cout<<"Found ["<<exe->filename<<"] at ["<<HEX(exe->start)<<"]"<<endl;
    cout<<"Found ["<<libc->filename<<"] at ["<<HEX(libc->start)<<"]"<<endl;
    cout<<"Found ["<<ld->filename<<"] at ["<<HEX(ld->start)<<"]"<<endl;
    
    /// Find region of address (it's so slow)
    region_t *roa = h->getRegionOfAddress(ld->start+8);
    cout<<"Region of address is: "<<roa->filename.c_str()<<endl;
    
stage_scanning:;
    size_t rescanned = 0;
    Scanner sc(h);
    
    // Known variables
    scan_data_type_t data_type;
    string data_type_s;
    
    cout<<"Enter data type: (a(ii(c,s,i,l), ff(f,d)), b, s): ";
    cin>>data_type_s;
    if (cin.fail()) {
        cout<<""<<endl;
        cin.clear(); // clear error state
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard 'bad' character(s)
    }
    if      (data_type_s == "ii")   data_type = ANYINTEGER;
    else if (data_type_s == "ff")   data_type = ANYFLOAT;
    else if (data_type_s == "c")    data_type = INTEGER8;
    else if (data_type_s == "s")    data_type = INTEGER16;
    else if (data_type_s == "i")    data_type = INTEGER32;
    else if (data_type_s == "l")    data_type = INTEGER64;
    else if (data_type_s == "f")    data_type = FLOAT32;
    else if (data_type_s == "d")    data_type = FLOAT64;
    else if (data_type_s == "b")    data_type = BYTEARRAY;
    else if (data_type_s == "s")    data_type = STRING;
    else                            data_type = ANYNUMBER;
    
    cout<<"Enter pattern: ";
    string text;
    cin>>text;
    
    
    // Unknown variables
    uservalue_t uservalue[2];
    scan_match_type_t match_type;
    try {
        // Parse text as uservalue
        sc.string_to_uservalue(data_type, text, &match_type, uservalue);
    } catch (bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        goto stage_scanning;
    }
    
    // Clock for performance report
    high_resolution_clock::time_point t1, t2;
    duration<double> time_span;
    
    // Overall time counted for both methods
    static duration<double> counterA = high_resolution_clock::duration::zero();
    static duration<double> counterB = high_resolution_clock::duration::zero();
    
    /// Now, most interesting begin
    // HackMe contains about 333274 (0x515DA) matches, so it will be 0x80000
    const size_t RESERVED = static_cast<size_t>(pow(2, ceil(log2(333274))));
    
    
    // Method A - sexy, but slow.
stage_rescanning:;
    if (!h->isRunning())
        goto stage_waiting;
    
    // Now create an array to store our matches
    vector<match> matches;
    matches.reserve(RESERVED);
    
    
    uint8_t *buffer = nullptr;
    uintptr_t r_off = 0;
    uintptr_t totalsize = 0;
    size_t step = 1;
    
#define PART1 \
    for(region_t &r : h->regions) {                                                                                         \
        if (!r.writable || !r.readable) continue;                                                                           \
        totalsize = r.end - r.start; /* calculate size of region */                                                         \
        delete [] buffer; buffer = new uint8_t[totalsize];  /* reallocate buffer */                                         \
        if (!h->read(buffer, r.start, totalsize)) { clog<<"error: invalid region: cant read memory: "<<r<<endl; continue; } \
        for(r_off = 0; ;  r_off += step, totalsize -= step) {                                                               \
            match m (r.start + r_off, *reinterpret_cast<mem64_t *>(&buffer[r_off]));
#define PART2 \
            if (totalsize >= 8) {                                                                                           \
                if (m.flags)                                                                                                \
                    matches.push_back(m);                                                                                   \
            }                                                                                                               \
            else if (totalsize >= 4) {                                                                                      \
                m.flags &= ~(flags_64b);                                                                                    \
                if (m.flags)                                                                                                \
                    matches.push_back(m);                                                                                   \
            }                                                                                                               \
            else if (totalsize >= 2) {                                                                                      \
                m.flags &= ~(flags_64b | flags_32b);                                                                        \
                if (m.flags)                                                                                                \
                    matches.push_back(m);                                                                                   \
            }                                                                                                               \
            else {                                                                                                          \
                m.flags &= ~(flags_64b | flags_32b | flags_16b);                                                            \
                if (m.flags)                                                                                                \
                    matches.push_back(m);                                                                                   \
            }                                                                                                               \
            if (totalsize - step >= totalsize)                                                                              \
                break;                                                                                                      \
        }                                                                                                                   \
    }
    
    t1 = high_resolution_clock::now();
    if (data_type == BYTEARRAY) {
        clog<<"not supported"<<endl;
    }
    else if (data_type == STRING) {
        clog<<"not supported"<<endl;
    } else {
        switch(match_type) {
                case MATCHANY:
                    PART1
                            m.flags = flags_all;
                    PART2
                    break;
                case MATCHEQUALTO:
                    PART1
                            m.flags = flags_empty;
                            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);
                            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
                    PART2
                    break;
                case MATCHNOTEQUALTO:
                    PART1
                            m.flags = flags_empty;
                            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   != uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  != uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  != uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  != uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value != uservalue[0].float32_value)) m.flags |= (flag_f32b);
                            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value != uservalue[0].float64_value)) m.flags |= (flag_f64b);
                    PART2
                    break;
                case MATCHGREATERTHAN:
                    PART1
                            m.flags = flags_empty;
                            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   >  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  >  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  >  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  >  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value >  uservalue[0].float32_value)) m.flags |= (flag_f32b);
                            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value >  uservalue[0].float64_value)) m.flags |= (flag_f64b);
                    PART2
                    break;
                case MATCHLESSTHAN:
                    PART1
                            m.flags = flags_empty;
                            if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   <  uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                            if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  <  uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                            if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  <  uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                            if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  <  uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                            if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value <  uservalue[0].float32_value)) m.flags |= (flag_f32b);
                            if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value <  uservalue[0].float64_value)) m.flags |= (flag_f64b);
                    PART2
                    break;
                case MATCHRANGE:
                    PART1
                            m.flags = flags_empty;
                            if ((uservalue[0].flags & flags_i8b ) && (uservalue[0].uint8_value   <= m.memory.uint8_value   ) && (m.memory.uint8_value   >= uservalue[1].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);
                            if ((uservalue[0].flags & flags_i16b) && (uservalue[0].uint16_value  <= m.memory.uint16_value  ) && (m.memory.uint16_value  >= uservalue[1].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);
                            if ((uservalue[0].flags & flags_i32b) && (uservalue[0].uint32_value  <= m.memory.uint32_value  ) && (m.memory.uint32_value  >= uservalue[1].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);
                            if ((uservalue[0].flags & flags_i64b) && (uservalue[0].uint64_value  <= m.memory.uint64_value  ) && (m.memory.uint64_value  >= uservalue[1].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);
                            if ((uservalue[0].flags & flag_f32b ) && (uservalue[0].float32_value <= m.memory.float32_value ) && (m.memory.float32_value >= uservalue[1].float32_value)) m.flags |= (flag_f32b);
                            if ((uservalue[0].flags & flag_f64b ) && (uservalue[0].float64_value <= m.memory.float64_value ) && (m.memory.float64_value >= uservalue[1].float64_value)) m.flags |= (flag_f64b);
                    PART2
                    break;
                default:
                    clog<<"error: only first_scan supported"<<endl;
                    goto stage_scanning;
            }
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    counterB += time_span;
    cout<<"Method B: done "<<matches.size()<<" matches, in: "<<time_span.count()<<" seconds, overall: "<<counterB.count()<<" seconds."<<endl;
    
    if (matches.capacity() > RESERVED) {
        cout<<"warning: matches.capacity() > RESERVED: matches.capacity(): "<<matches.capacity()<<endl;
    }
    
    // Method B - fastest, but with crappy macros
    matches.clear();
    matches.shrink_to_fit();
    matches.reserve(RESERVED);
    if (rescanned++ < 100)
        goto stage_rescanning;
    else
        goto stage_scanning;
    
/// other example
//    vector<uintptr_t> sig;
//    size_t found = h->findPattern(&sig, ld,
//                   "\x48\x89\xC7\xE8\x00\x00\x00\x00",
//                   "xxxx????");
//    clog<<"found: "<<found<<endl;
//    if (found > 0 && found < 20)
//        for(uintptr_t a : sig) { 
//            clog<<"\tat 0x"<<HEX(a)<<endl;
//        }
    
/// idk what is it
//    size_t len = 32;
//    void *v = new char[len];
//    clog<<"v in hex: "<<HEX(*(int *)(v))<<endl;
//    string s((char *)v,len);
//    clog<<"unconverted s: "<<HEX(*(int *)(v))<<endl;
//
//    static const char* const lut = "0123456789ABCDEF";
//    string output;
//    output.reserve(2 * len);
//
//    for(int i=0; i<len; i++) {
//        const unsigned char c = s[i];
//        output.push_back(lut[c >> 4]);
//        output.push_back(lut[c & 15]);
//    }
//
//    clog<<"output: "<<output<<endl;
    
    return 0;
}
#pragma clang diagnostic pop


























