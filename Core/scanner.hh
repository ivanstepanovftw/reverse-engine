//
// Created by root on 09.03.18.
//

#ifndef RE_SCANNER_HH
#define RE_SCANNER_HH

#include "core.hh"
#include "inttypes.h"
#include "value.hh"
#include <memory>
#include <functional>
#include <zlib.h>
#include <cmath>
#include <bitset>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <chrono>
#include <mutex>
#include <deque>
#include <string>
#include <thread>
#include <condition_variable>
#include <boost/iostreams/device/mapped_file.hpp>


namespace bio = boost::iostreams;

using namespace std;

#pragma pack(push, 1)
class match_t {
public:
    uintptr_t address;      //+8=8
    mem64_t memory;         //+8=16
//    mem64_t memory_old;     //+8=24
    uint16_t flags;         //+2=26 or 18 w/o memory_old
    
    explicit match_t() {
        this->address = 0;
//        this->memory.uint64_value = 0;
        this->flags = 0;
    }
    
    explicit match_t(uintptr_t address, mem64_t memory, uint16_t userflag = flags_empty) {
        this->address = address;
//        this->memory = memory;
        this->flags = userflag;
    }
};
#pragma pack(pop)



class matches_t {
//    todo 222 , мы не обязаны флушить в этом участке кода. добавь булеан в сам класс и пусть проверяет,
// флушали ли мы уже или нет. todo а ещё пусть там mmap будет, а не ifstream
public:
    void open() {
        f.open("ADDRESSES.FIRST", ios::in | ios::out | ios::binary | ios::trunc);
    }
    
    matches_t() {
        buffer = new char[sizeof(match_t)];
        this->open();
        mm.reserve(18*1024/sizeof(match_t));  // 18 KiB = 0x4800 = 1024 match_t
    }
    
    ~matches_t() {
        delete buffer;
        clear();
        f.close();
    }
    
    // accessor
    match_t get(fstream::off_type index) {
        f.seekg(index*sizeof(match_t), ios::beg);
        f.read(buffer, sizeof(match_t));
        return *reinterpret_cast<match_t *>(buffer);
    }
    
    void flush() {
        char *b = reinterpret_cast<char *>(&mm[0]);
        f.seekp(0, ios::end);
        f.write(b, mm.size()*sizeof(match_t));
        mm.clear();
    }
    
    // mutator
    void set(fstream::off_type index, const match_t& m) {
        f.seekp(index * sizeof(match_t), ios::beg);
        f.write(reinterpret_cast<const char *>(&m), sizeof(match_t));
    }
    
    // mutator
    void append(const match_t& m) {
        if (mm.size() >= mm.capacity())
            flush();
        mm.push_back(m);
    }
    
    // mutator
    void append(const vector<match_t>& ms) {
        flush();
        for(size_t i = 0; i<ms.size(); i++) {
            this->append(ms);
        }
    }
    
    fstream::pos_type size() {
        return f.seekg(0, ios::end).tellg()/sizeof(match_t);
    }
    
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


class bad_uservalue_cast
{
    std::string _text;   // оригинальная строка
    std::size_t _at;     // указывает, на каком символе ошибка

public:
    bad_uservalue_cast(const std::string &original, const std::size_t &at) _GLIBCXX_USE_NOEXCEPT {
        _text = original;
        _at = at;
    }
    
    // This declaration is not useless: http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
    ~bad_uservalue_cast() _GLIBCXX_USE_NOEXCEPT { }
    
    const std::string what() const _GLIBCXX_USE_NOEXCEPT {
        return ("bad_uservalue_cast(): \n"
                + _text + "\n"
                + std::string(_at, ' ') + "^" + std::string(_text.size() - _at - 1, '~'));
    };
    
    const std::string text() _GLIBCXX_USE_NOEXCEPT {
        return _text;
    };
    
    std::size_t at() const _GLIBCXX_USE_NOEXCEPT {
        return _at;
    };
};


const string snapshot_mf_path = "MEMORY.TMP";

class Scanner
{
public:
    /// Create file for storing matches
    matches_t matches;
    
    Handle *handle;
    bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
    
    size_t scan_count = 0;
    
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    boost::iostreams::mapped_file make_snapshot(const string &path);
    
    void string_to_uservalue(const scan_data_type_t &data_type,
                             const std::string &text,
                             scan_match_type_t *match_type,
                             uservalue_t *vals);
    
    
    bool scan(const scan_data_type_t data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t match_type);
    
    bool scan(const bio::mapped_file& snapshot_mf,
              const scan_data_type_t data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t match_type);
    
    bool scan_reset();
    
//    bool update();
    
//    bool sm_choose_scanroutine(scan_data_type_t dt, scan_match_type_t mt, const uservalue_t *uval, bool reverse_endianness);
    
    /* This is the function that handles when you enter a value (or >, <, =) for the second or later time (i.e. when there's already a list of matches);
     * it reduces the list to those that still match_t. It returns false on failure to attach, detach, or reallocate memory, otherwise true. */
    //    bool sm_checkmatches(scan_match_type_t match_type, const uservalue_t *uservalue);
    
    
};

#endif //RE_SCANNER_HH
