//
// Created by root on 12.02.18.
//

#include <iostream>
#include <iomanip>
#include <chrono>
#include <type_traits>
#include <zconf.h>
#include <functional>
#include <cmath>
#include <cassert>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/param.h> //MIN, MAX

#include <boost/iostreams/device/mapped_file.hpp>

#include <Core/core.hh>
#include <Core/value.hh>


#define HEX(s) hex<<showbase<<(s)<<dec

#define CHECK(x) { if(!(x)) { \
fprintf(stderr, "%s:%i: failure at: %s\n", __FILE__, __LINE__, #x); \
_exit(1); } }


using namespace std;
using namespace std::chrono;
using namespace std::literals;
namespace bio = boost::iostreams;


const char *target = "FakeMem";
const char *to_find = "0";
const char *snapshot_path = "MEMORY.TMP";
//scan_data_type_t data_type = INTEGER64;
scan_data_type_t data_type = INTEGER8;
//scan_data_type_t data_type = ANYNUMBER;
constexpr size_t step = 1;

class task_queue_t
{
public:
    typedef function<void(char *)> task_t;
    vector<thread> pool_m;
    deque<task_t> deque_m;
    condition_variable condition_m;
    mutex mutex_m;
    atomic<bool> done_m{false};
    
    task_queue_t(size_t region_length, size_t pool_size = thread::hardware_concurrency())
    {
        pool_m.reserve(pool_size);
        for(size_t i = 0; i < pool_size; i++)
            pool_m.emplace_back(bind(&task_queue_t::worker, this, region_length, i));
    }
    
    ~task_queue_t()
    {
        join_all();
    }
    
    void join_all()
    {
        unique_lock<mutex> lock{mutex_m};
        if (done_m.exchange(true))
            return;
        condition_m.notify_all();
        lock.unlock();
        for(auto& thread : pool_m)
            thread.join();
    }
    
    template<typename F>
    void push(F&& function)
    {
        deque_m.emplace_back(forward<F>(function));
    }

private:
    void worker(size_t region_length, size_t i)
    {
        task_t task;
        char buffer[region_length];
        
        {
            unique_lock<mutex> lock{mutex_m};
            condition_m.wait(lock, [=]() { return !!done_m; });
            clog<<"started "<<i<<endl;
        }
        
        while (true) {
            unique_lock<mutex> lock{mutex_m};
            if (deque_m.empty())
                break;
            task = deque_m.front();
            deque_m.pop_front();
            lock.unlock();
            
            task(buffer);
        }
    }
    
    task_queue_t(const task_queue_t &) = delete;
    task_queue_t(task_queue_t &&) = delete;
    task_queue_t &operator=(const task_queue_t &) = delete;
    task_queue_t &operator=(task_queue_t &&) = delete;
};


#pragma pack(push, 1)
class match_t {
public:
    uintptr_t address;      //+8=8
    mem64_t memory;         //+8=16
//    mem64_t memory_old;     //+8=24
    uint16_t flags;         //+2=26 or 18 w/o memory_old
    
    match_t() {
        this->address = 0;
        this->memory.uint64_value = 0;
        this->flags = 0;
    }
    
    explicit match_t(uintptr_t address, mem64_t memory, uint16_t userflag = flags_empty) {
        this->address = address;
        this->memory = memory;
        this->flags = userflag;
    }
};
#pragma pack(pop)


class matches_t {
public:
    inline
    void open() {
        f.open("ADDRESSES.FIRST", ios::in | ios::out | ios::binary | ios::trunc);
    }
    
    matches_t() {
        buffer = new char[sizeof(match_t)];
        open();
        mm.reserve(0x0F'00'00'00/sizeof(match_t));  // 240 MiB
    }
    
    ~matches_t() {
        clear();
        f.close();
        delete buffer;
    }
    
    // accessor
    inline
    match_t get(fstream::off_type index) {
        f.seekg(index*sizeof(match_t), ios::beg);
        f.read(buffer, sizeof(match_t));
        return *reinterpret_cast<match_t *>(buffer);
    }
    
    inline
    void flush() {
        char *b = reinterpret_cast<char *>(&mm[0]);
        f.seekp(0, ios::end);
        f.write(b, sizeof(match_t)*mm.size());
        mm.clear();
    }
    
    // mutator
    inline
    void set(fstream::off_type index, match_t& m) {
        f.seekp(index * sizeof(match_t), ios::beg);
        f.write(reinterpret_cast<char *>(&m), sizeof(match_t));
    }
    
    inline
    void append(match_t& m) {
        if (mm.size() >= mm.capacity())
            flush();
        mm.push_back(m);
    }
    
    inline
    fstream::pos_type size() {
        return f.seekg(0, ios::end).tellg()/sizeof(match_t);
    }
    
    inline
    void clear() {
        mm.clear();
        f.close();
        open();
    }

private:
    vector<match_t> mm;
    fstream f;
    char *buffer;
};


int
main() {
    Handle handle(target);
    CHECK(handle.is_running())
    handle.update_regions();
    
    uservalue_t uservalue[2];
    scan_match_type_t match_type;
    parse_uservalue_number(to_find, &uservalue[0]);
    uservalue[0].flags &= scan_data_type_to_flags[data_type];
    uservalue[1].flags &= scan_data_type_to_flags[data_type];
    
    /// Make snapshot
    /// Allocate space
    uintptr_t total_scan_bytes = 0;
    for(const region_t &region : handle.regions)
        total_scan_bytes += region.end - region.address;
    
    CHECK(total_scan_bytes > 0);
    
    /// Create mmap
    bio::mapped_file_params snapshot_mf_;
    snapshot_mf_.path          = snapshot_path;
    snapshot_mf_.flags         = bio::mapped_file::mapmode::readwrite;
    snapshot_mf_.offset        = 0;
    snapshot_mf_.length        = total_scan_bytes;
    snapshot_mf_.new_file_size = total_scan_bytes;
    bio::mapped_file snapshot_mf(snapshot_mf_);
    CHECK(snapshot_mf.is_open());
    
    char *snapshot_begin = snapshot_mf.data();
    char *snapshot_end;
    char *snapshot;
    
    /// Snapshot goes here
    uintptr_t region_size = 0;
    uintptr_t cursor_dst = 0;
    
    chrono::high_resolution_clock::time_point timestamp = chrono::high_resolution_clock::now();
    
    for(region_t &region : handle.regions) {
        region_size = region.end - region.address;
        if (!region.writable || !region.readable || region_size <= 1)
            continue;
        
        memcpy((void *)snapshot_begin+cursor_dst, &region.address, 2*sizeof(region.address));
        cursor_dst += sizeof(region.address)+sizeof(region.end);
        
        if (!handle.read((void *)snapshot_begin+cursor_dst, region.address, region_size)) {
            clog<<"warning: region not copied: region: "<<region<<endl;
            cursor_dst -= sizeof(region.address)+sizeof(region.end);
            total_scan_bytes -= region_size;
            
            CHECK(handle.is_running());
        } else
            cursor_dst += region_size;
    }
    
    clog<<"Done "<<total_scan_bytes<<" bytes, in: "
        <<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    snapshot_mf.resize(cursor_dst);
    clog<<"Snapshot size: "<<snapshot_mf.size()<<" bytes."<<endl;
    
    snapshot_begin = snapshot_mf.data();
    snapshot_end = snapshot_begin + snapshot_mf.size();
    snapshot = snapshot_begin;
    
    
    /// Create file for storing matches
    matches_t matches;
    
    /// Scanning routines
    constexpr size_t region_shift = 2*sizeof(region_t::begin);
    constexpr size_t buffer_size = 4*1024*1024;  // 4 MiB
    region_t region;
    mutex getter_mutex;
    mutex putter_mutex;
    

#define FLAGS_CHECK_CODE\
    m.flags = flags_empty;\
    if ((uservalue[0].flags & flags_i8b ) && (m.memory.uint8_value   == uservalue[0].uint8_value  )) m.flags |= (uservalue[0].flags & flags_i8b);\
    if ((uservalue[0].flags & flags_i16b) && (m.memory.uint16_value  == uservalue[0].uint16_value )) m.flags |= (uservalue[0].flags & flags_i16b);\
    if ((uservalue[0].flags & flags_i32b) && (m.memory.uint32_value  == uservalue[0].uint32_value )) m.flags |= (uservalue[0].flags & flags_i32b);\
    if ((uservalue[0].flags & flags_i64b) && (m.memory.uint64_value  == uservalue[0].uint64_value )) m.flags |= (uservalue[0].flags & flags_i64b);\
    if ((uservalue[0].flags & flag_f32b ) && (m.memory.float32_value == uservalue[0].float32_value)) m.flags |= (flag_f32b);\
    if ((uservalue[0].flags & flag_f64b ) && (m.memory.float64_value == uservalue[0].float64_value)) m.flags |= (flag_f64b);
    
    match_t m;
    
    auto call = [&](char *buffer, char *map, size_t size) -> void {
        {
            scoped_lock<mutex> lock{getter_mutex};
            memcpy(reinterpret_cast<void *>(buffer), reinterpret_cast<void *>(map), size);
            clog<<"memcpy("<<HEX(reinterpret_cast<void *>(buffer))
                <<", "<<HEX(reinterpret_cast<void *>(map))
                <<", "<<HEX(size)<<"): "<<endl;
        }
        
        char *buffer_end = buffer + size - 7;
        
        while (buffer < buffer_end) {
            m.memory = *reinterpret_cast<mem64_t *>(buffer);
            FLAGS_CHECK_CODE
            if (m.flags) {
                scoped_lock<mutex> lock{putter_mutex};
                matches.append((m));
            }
            buffer += step;
        }
        buffer_end += 4;
        while (buffer < buffer_end) {
            m.memory = *reinterpret_cast<mem64_t *>(buffer);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b);
            if (m.flags) {
                scoped_lock<mutex> lock{putter_mutex};
                matches.append((m));
            }
            buffer += step;
        }
        buffer_end += 2;
        while (buffer < buffer_end) {
            m.memory = *reinterpret_cast<mem64_t *>(buffer);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b);
            if (m.flags) {
                scoped_lock<mutex> lock{putter_mutex};
                matches.append((m));
            }
            buffer += step;
        }
        buffer_end += 1;
        while (buffer < buffer_end) {
            m.memory = *reinterpret_cast<mem64_t *>(buffer);
            FLAGS_CHECK_CODE
            m.flags &= ~(flags_64b | flags_32b | flags_16b);
            if (m.flags) {
                scoped_lock<mutex> lock{putter_mutex};
                matches.append((m));
            }
            buffer += step;
        }
    };
    
    clog<<"parsing"<<endl;
    
    timestamp = chrono::high_resolution_clock::now();
    task_queue_t pool(buffer_size+7, 1);
    size_t calls = 0;
    
    while(snapshot < snapshot_end) {
        memcpy(reinterpret_cast<void*>(&region.address), snapshot, sizeof(region_t::begin));
        snapshot += sizeof(region_t::begin);
        memcpy(reinterpret_cast<void*>(&region.end), snapshot, sizeof(region_t::end));
        snapshot += sizeof(region_t::end);
        
        region_size = region.end - region.address;
        clog<<"region_size: "<<region_size<<endl;
        
        while (region_size > buffer_size) {
            pool.push(bind(call, placeholders::_1, snapshot, buffer_size+7));
            snapshot += buffer_size;
            region_size -= buffer_size;
            calls++;
        }
        pool.push(bind(call, placeholders::_1, snapshot, region_size));
        snapshot += region_size;
        calls++;
    }
    
    clog<<"we made "<<calls<<" calls."<<endl;
    pool.join_all();
    
    clog<<"force flushing"<<endl;
    matches.flush();
    
    clog<<"Done "<<matches.size()<<" matches, in: "
        <<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - timestamp).count()<<" seconds."<<endl;
    
    return 0;
}