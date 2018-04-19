//
// Created by root on 09.03.18.
//

#include "scanner.hh"

#define HEX(s) hex<<showbase<<(s)<<dec

using namespace std;
using namespace boost::iostreams;


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


mapped_file
Scanner::snapshot(const string &path)
{
    /// Allocate space
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : handle->regions)
        total_scan_bytes += region.end - region.start;
    
    if (total_scan_bytes == 0) {
        if (handle->regions.size() == 0)
            throw invalid_argument("no regions defined, perhaps you deleted them all?");
        throw invalid_argument("no bytes to snapshot");
    }
    
    /// Create mmap
    mapped_file_params snapshot_mf_;
    snapshot_mf_.path          = path;
    snapshot_mf_.flags         = mapped_file::mapmode::readwrite;
    snapshot_mf_.offset        = 0;
    snapshot_mf_.length        = total_scan_bytes;
    snapshot_mf_.new_file_size = total_scan_bytes;
    
    mapped_file snapshot_mf(snapshot_mf_);
    if (!snapshot_mf.is_open())
        throw invalid_argument("cant open snapshot_mf");
    
    char *data = snapshot_mf.data();
    
    /// Snapshot goes here
    uintptr_t region_size = 0;
    uintptr_t cursor_dst = 0;
    
    chrono::high_resolution_clock::time_point timestamp = chrono::high_resolution_clock::now();
    
    for(region_t &region : handle->regions) {
        region_size = region.end - region.start;
        
        memcpy(data+cursor_dst, &region.start, sizeof(region.start)+sizeof(region.end));
        cursor_dst += sizeof(region.start)+sizeof(region.end);
        
        if (!handle->read(data+cursor_dst, region.start, region_size)) {
            clog<<"warning: region not copied: region: "<<region<<endl;
            cursor_dst -= sizeof(region.start)+sizeof(region.end);
            total_scan_bytes -= region_size;
            
            if (!handle->isRunning()) {
                throw invalid_argument("process not running");
            }
        } else
            cursor_dst += region_size;
    }
    
    clog<<"Done "<<total_scan_bytes<<" bytes, in: "
        <<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    snapshot_mf.resize(cursor_dst);
    clog<<"Snapshot size: "<<snapshot_mf.size()<<" bytes."<<endl;
    
    return snapshot_mf;
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
    string snapshot_mf_path = "MEMORY.TMP";
    mapped_file snapshot_mf;
    try {
        snapshot_mf = snapshot(snapshot_mf_path);
    } catch (exception &e) {
        clog<<"what(): "<<e.what()<<endl;
        return false;
    }
    
    
#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
    
    
    
    /// Create file for storing matches
    string mathces_mf_path = "ADDRESSES.TMP";
    
    mapped_file_params matches_mf_;
    matches_mf_.path          = mathces_mf_path;
    matches_mf_.flags         = mapped_file::mapmode::readwrite;
    matches_mf_.offset        = 0;
    matches_mf_.length        = snapshot_mf.size()*sizeof(match);
    matches_mf_.new_file_size = snapshot_mf.size()*sizeof(match);
    
    mapped_file matches_mf(matches_mf_);
    if (!matches_mf.is_open()) {
        clog<<"could not map the '"<<mathces_mf_path<<"'"<<endl;
        return false;
    }
    
    /// Scanning routines begins here
    
    char *snapshot_map = snapshot_mf.data();
    char *snapshot_map_end = snapshot_map + snapshot_mf.size() - 1;
    match *matches_map = reinterpret_cast<match *>(matches_mf.data());
    
    uintptr_t cursor = 0;
    uintptr_t total_bytes = snapshot_mf.size();
    size_t region_size;
    region_t region;
    match m;
    
    constexpr size_t region_shift = 2*sizeof(region_t::start);
    constexpr size_t buffer_size = 4*1024*1024;
    
    boost::mutex getter_mutex;
    boost::mutex putter_mutex;
    
    chrono::high_resolution_clock::time_point timestamp = chrono::high_resolution_clock::now();
    /// todo возвращаемся к версии нормальной, эту оставляем.
    /// а это означает, что создаётся 8 потоков, каждый имеет свой оффсет. 
    /** create thread pool */
#define PART1(FLAGS_CHECK_CODE)
    {
        boost::asio::io_service io_service;
        boost::thread_group threads;
        {
            boost::scoped_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
            for(size_t t = 0; t < boost::thread::hardware_concurrency(); t++) {
                threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
            }
            
            function call = [&](size_t cursor, size_t size) -> void {
                char *bytes_beg = new char[size];
                {
                    boost::mutex::scoped_lock getter_lock(getter_mutex);
                    memcpy(bytes_beg, snapshot_map+cursor, size);
                }
    
                char *bytes_cur = bytes_beg;
                char *bytes_end = bytes_cur + size;
                
                while (bytes_cur < bytes_end) {
                    m.memory = *reinterpret_cast<mem64_t *>(bytes_cur);
                    FLAGS_CHECK_CODE
                    if (m.flags)
                    {
                        boost::mutex::scoped_lock putter_lock(putter_mutex);
                        *matches_map++ = m;
                    }
                    bytes_cur += step;
                }
                bytes_end += 4;
                while (bytes_cur < bytes_end) {
                    m.memory = *reinterpret_cast<mem64_t *>(bytes_cur);
                    FLAGS_CHECK_CODE
                    m.flags &= ~(flags_64b);
                    if (m.flags)
                    {
                        boost::mutex::scoped_lock putter_lock(putter_mutex);
                        *matches_map++ = m;
                    }
                    bytes_cur += step;
                }
                bytes_end += 2;
                while (bytes_cur < bytes_end) {
                    m.memory = *reinterpret_cast<mem64_t *>(bytes_cur);
                    FLAGS_CHECK_CODE
                    m.flags &= ~(flags_64b | flags_32b);
                    if (m.flags)
                    {
                        boost::mutex::scoped_lock putter_lock(putter_mutex);
                        *matches_map++ = m;
                    }
                    bytes_cur += step;
                }
                bytes_end += 1;
                while (bytes_cur < bytes_end) {
                    m.memory = *reinterpret_cast<mem64_t *>(bytes_cur);
                    FLAGS_CHECK_CODE
                    m.flags &= ~(flags_64b | flags_32b | flags_16b);
                    if (m.flags)
                    {
                        boost::mutex::scoped_lock putter_lock(putter_mutex);
                        *matches_map++ = m;
                    }
                    bytes_cur += step;
                }
                delete [] bytes_beg;
            };
            
            /** tasks queue */
            {
                boost::mutex::scoped_lock getter_lock(getter_mutex);
                while(cursor < total_bytes) {
                    memcpy(&region.start, snapshot_map+cursor, region_shift);
                    cursor += region_shift;
                    region_size = region.end - region.start;
                    io_service.post(boost::bind(call, cursor, MIN(region_size, buffer_size+7)));
                    cursor += MIN(region_size, buffer_size);
                }
            }
        }
        threads.join_all();
    }
    
    
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

Using buffered reader & writer
 (fakemem 428 MiB):
Done 449150976 bytes, in: 2.07514 seconds.
Done 449128277 matches, in: 81.3764 seconds.
Done 449128277 matches, in: 69.7533 seconds.
Done 449128277 matches, in: 75.5361 seconds.
 
Using mmap
 (fakemem 428 MiB):
Done 449114593 matches, in: 56.0347 seconds.
Done 448996950 matches, in: 55.885 seconds.
todo добавить мультипоточность и ещё как мультипоточность добавить при компиляции? я слышал что-то типа такого
 
 
 (fakemem 111 MiB):
Done 116752384 bytes, in: 0.125883 seconds.
Done 116716123 matches, in: 13.3575 seconds.
Done 116598353 matches, in: 16.7437 seconds.    CPU.throttling.enable()
Done 116598353 matches, in: 15.5291 seconds.
Done 116598353 matches, in: 13.012 seconds.     CPU.throttling.disable()
Done 116598353 matches, in: 13.2313 seconds.    yeah that was throttling
Done 116729925 matches, in: 7.0059 seconds.

 (fakemem 33 MiB):
Done 34797174 matches, in: 2.35997 seconds. (debug)
Done 34797174 matches, in: 2.19174 seconds.
 
Using boost
 (fakemem 111 MiB)
Done 116617216 bytes, in: 0.168937 seconds.
Done 116617216 bytes, in: 0.158683 seconds.
Done 116600875 matches, in: 9.76095 seconds.
Done 116600875 matches, in: 8.54763 seconds.
Done 116600875 matches, in: 9.40879 seconds.
 
 (fakemem 428 MiB):
Done 448999467 matches, in: 58.3596 seconds.
Done 448999467 matches, in: 67.3868 seconds.
 
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
    
    clog<<"Done "<<matches_map-reinterpret_cast<match *>(matches_mf.data())<<" matches, in: "
        <<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    
    /// В планах оставить сканирование без mmaping'а для самых счастливых
    /*
    uintptr_t b;
    vector<match> mm;
    
    
    timestamp = chrono::high_resolution_clock::now();
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
        <<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
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
/*    uservalue_t vals[2];
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
    
    this->scan_progress = 1.0;*/
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
