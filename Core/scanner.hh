//
// Created by root on 09.03.18.
//

//#pragma clang diagnostic push
//#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef RE_SCANNER_HH
#define RE_SCANNER_HH

#include "core.hh"
#include "inttypes.h"
#include "value.hh"
#include <memory>
#include <functional>
#include <zlib.h>
#include <cmath>

using namespace std;

#pragma pack(push, 1)
class match {
public:
    uintptr_t address;      //+8=8
    mem64_t memory;         //+8=16
    mem64_t memory_old;     //+8=24
    uint16_t flags;         //+2=26
    
    explicit match(uintptr_t address, mem64_t memory, match_flags userflag = flags_empty) {
        this->address = address;
        this->memory = memory;
        this->flags = userflag;
    }
    
    const char *data() {
        return reinterpret_cast<const char *>(this);
    }
    
    static size_t size() {
        return sizeof(match);
    }
};
#pragma pack(pop)

using namespace std;

/*
class ram_compressed_matches : public matches {
    vector<uint8_t *> _comp;
    
    string compress(vector<match> &in, int compressionlevel = Z_BEST_COMPRESSION) {
        z_stream zs;                        // z_stream is zlib's control structure
        memset(&zs, 0, sizeof(zs));
        
        if (deflateInit(&zs, compressionlevel) != Z_OK)
            throw(std::runtime_error("deflateInit failed while compressing."));
        
        vector<uint8_t> tmp;
        tmp.reserve(in.size()*sizeof(match));
        
        for(match &b : in) {
            uint8_t *data = b.data();
            for(int i = 0; i < b.size(); i++) {
                tmp.push_back(data[i]);
            }
        }
        
        zs.next_in = tmp.data();
        zs.avail_in = static_cast<uInt>(tmp.size());
        
        int ret;
        char outbuffer[32768];
        std::string outstring;
        
        // retrieve the compressed bytes blockwise
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
        
            ret = deflate(&zs, Z_FINISH);
        
            if (outstring.size() < zs.total_out) {
                // append the block to the output string
                outstring.append(outbuffer,
                                 zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);
        
        deflateEnd(&zs);
        
        if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
            std::ostringstream oss;
            oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
            throw(std::runtime_error(oss.str()));
        }
        
        return outstring;
    }
    
    
public:
    compressed_matches() {
    }
    
};
*/


/**
 * В CE создаётся три файла:
 * MEMORY.TMP - это снепшот
 * ADDRESSES.FIRST - самый первый
 * ADDRESSES.TMP - создаётся после next_scan
 * */
class file_matches {
public:
    inline
    void open() {
        f.open("ADDRESSES.FIRST", ios::in | ios::out | ios::binary | ios::trunc);
    }
    
    file_matches() {
        buffer = new char[match::size()];
        open();
        mm.reserve(0x40'00'00/match::size());  // (128 mb)
    }
    
    ~file_matches() {
        clear();
        f.close();
        delete buffer;
    }
    
    // accessor
    inline
    match get(fstream::off_type index) {
        f.seekg(index*match::size(), ios::beg);
        f.read(buffer, match::size());
        return *reinterpret_cast<match *>(buffer);
    }
    
    inline
    void flush() {
        char *b = reinterpret_cast<char *>(&mm[0]);
        f.seekp(0, ios::end);
        f.write(b, match::size()*mm.size());
        mm.clear();
    }
    
    // mutator
    inline
    void set(fstream::off_type index, match m) {
        f.seekp(index * match::size(), ios::beg);
        f.write(m.data(), match::size());
    }
    
    inline
    void append(match &m) {
        if (mm.size() >= mm.capacity())
            flush();
        mm.push_back(m);
    }
    
    inline
    void append(vector<match> &ms) {
        flush();
        for(size_t i = 0; i<ms.size(); i++) {
            append(ms);
        }
//        char *b = reinterpret_cast<char *>(&mm[0]);
//        f.seekp(0, ios::end);
//        f.write(b, match::size()*mm.size());
    }
    
    inline
    fstream::pos_type size() {
        return f.seekg(0, ios::end).tellg()/match::size();
    }
    
    inline
    void clear() {
        mm.clear();
        f.close();
        open();
    }
    
private:
    vector<match> mm;
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

class Scanner
{
public:
    fstream fsnapshot;
    file_matches matches;
    
    Handle *handle;
    bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
        
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    /** 
     * @throws 
     * @return 
     */
    bool snapshot();
    
    void string_to_uservalue(const scan_data_type_t &data_type, const std::string &text,
                             scan_match_type_t *match_type, uservalue_t *vals);
    
    
    bool first_scan(const scan_data_type_t data_type, uservalue_t *uservalue, const scan_match_type_t match_type);
    
    bool next_scan(scan_data_type_t data_type, const std::string &text);
    
//    bool update();
    
//    bool sm_choose_scanroutine(scan_data_type_t dt, scan_match_type_t mt, const uservalue_t *uval, bool reverse_endianness);
    
    /* This is the function that handles when you enter a value (or >, <, =) for the second or later time (i.e. when there's already a list of matches);
     * it reduces the list to those that still match. It returns false on failure to attach, detach, or reallocate memory, otherwise true. */
    //    bool sm_checkmatches(scan_match_type_t match_type, const uservalue_t *uservalue);
    
    
};

#endif //RE_SCANNER_HH

//#pragma clang diagnostic pop





















