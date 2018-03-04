//
// Created by root on 12.02.18.
//

#ifndef HACKME_SDK_HH
#define HACKME_SDK_HH

#define HEX(a) ("0x")<<hex<<(a)<<dec

#include <sys/uio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>


enum ErrorCode { //todo #54 error codes and fn like dlError();
//    catch (const Glib::Error &e) {
//        std::cerr << e.what() << std::endl;
//        return 1;
//    }
};

using namespace std;

std::vector<std::string> split(const std::string &text, const std::string &delims, uint64_t d = UINT64_MAX);

std::string execute(std::string cmd);

std::vector<std::vector<std::string>> getProcesses();

//typedef uint8_t byte;

typedef struct {
    // Memory
    uintptr_t start;
    uintptr_t end;

    // Permissions
    bool readable;
    bool writable;
    bool executable;
    bool shared;

    // File data
    uintptr_t offset;
    unsigned char deviceMajor;
    unsigned char deviceMinor;
    unsigned long inodeFileNumber;
    std::string pathname;
    std::string filename;
} region_t;



/** Scanner implementation */
enum ScanType {
    NUMBER,
    AOB,
    STRING,
    GROUPED,
};

enum ValueType {
    ALL,
    CHAR,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
};

class AddressEntry {
public:
    ValueType type;
    union {
        byte a[sizeof(uintptr_t)];
        uintptr_t n;
    } address;
    const region_t *region;
    
    AddressEntry(ValueType type, uintptr_t address, const region_t *region) // NOLINT
    {
        this->type = type;
        this->address.n = address;
        this->region = region;
    }
};

class PatternEntry {
public:
    ValueType type;
    vector<byte> bytes;
    
    PatternEntry(ValueType type, const vector<byte> &bytes)
    {
        this->type = type;
        this->bytes = bytes;
    }
};


class Handle {
public:
/// Variables
    pid_t pid;
    std::string pidStr;
    std::string title;
    std::vector<region_t> regions;

/// Costructors / Destrictors
    explicit Handle(pid_t pid);

    explicit Handle(const std::string &title);

/// Functions
    // Helpful functions
    std::string getSymbolicLinkTarget(const std::string &target);

    std::string getPath();

    std::string getWorkingDirectory();

    // Checking
    bool isValid();

    bool isRunning();

    // Read_from/write_to this handle
    bool inline read(void *out, void *address, size_t size);

    bool inline write(void *address, void *buffer, size_t size);

    // Modules
    void updateRegions();

    region_t *getRegion(const std::string &region_name = "");

    region_t *getModuleOfAddress(void *address);

    bool getCallAddress(uintptr_t *out, void *address);

    bool findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, 
                     size_t point_size = sizeof(uintptr_t), std::ostream *ss = nullptr);


    bool findPattern(uintptr_t *out, region_t *region, const char *pattern, const char *mask);
    
    size_t findPattern(vector<uintptr_t> *out, region_t *region, const char *pattern, const char *mask);
    
    //fixme 12345 implement "Value between", "Other than value" and "Unknown initial value"
    //fixme speedup it https://stackoverflow.com/questions/3664272/is-stdvector-so-much-slower-than-plain-arrays
    size_t scan_exact(vector<AddressEntry> *out,
                      const region_t *region,
                      vector<PatternEntry> patterns,  //attempt to implement Number scanner
                      size_t increment = 1);          //attempt to implement "Fast scan" (он же так работает?)
};


class HandleScanner {
public:
    Handle *handle;
    ScanType scan_type;
    
    ValueType value_type;
    size_t increment;
    
    double progress = 0;
    vector<AddressEntry> output;
    
    explicit HandleScanner(Handle *handle,
                           ScanType scan) 
            : handle(handle)
            , scan_type(scan)
    { }
    
    
    void setup_number_scan(ValueType valueType,
                           size_t increment,
                           int round = 0)
    {
        this->value_type = valueType;
        this->increment = increment;
    }
    
    
    void first_scan(const string &pattern)
    {
        output.clear();
        // using boost because http://www.boost.org/doc/libs/1_57_0/doc/html/boost_lexical_cast/performance.html
        if (scan_type == ScanType::NUMBER) { //fixme 12345
            vector<PatternEntry> patterns;
            //fixme find a way to simplify this code
            if (value_type == ValueType::ALL || value_type == ValueType::CHAR) {
                try {
                    auto b = boost::lexical_cast<int8_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::CHAR, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::SHORT) {
                try {
                    auto b = boost::lexical_cast<int16_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::SHORT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::INT) {
                try {
                    auto b = boost::lexical_cast<int32_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::INT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::LONG) {
                try {
                    auto b = boost::lexical_cast<int64_t>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::LONG, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::FLOAT) {
                try {
                    auto b = boost::lexical_cast<float>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::FLOAT, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            if (value_type == ValueType::ALL || value_type == ValueType::DOUBLE) {
                try {
                    auto b = boost::lexical_cast<double>(pattern);
                    auto a = reinterpret_cast<byte *>(&b);
                    patterns.emplace_back(ValueType::DOUBLE, vector<byte>(a, a+sizeof(b)));
                } 
                catch(boost::bad_lexical_cast &) { }
            }
            
            /// Scanning
            if (!patterns.empty()) {
                for(size_t i = 0; i < patterns.size(); i++)
                    printf("patterns[%li]: %x %x %x %x\n", i, 
                           patterns[i].bytes[0], patterns[i].bytes[1],
                           patterns[i].bytes[2], patterns[i].bytes[3]);
                
                for(int i = 0; i < handle->regions.size(); i++) {
                    region_t region = handle->regions[i];
                    if (region.writable) {
                        handle->scan_exact(&output, &region, patterns, increment);
                    }
                    progress = (i+1)/handle->regions.size();
                }
            }
        }
        else if (scan_type == ScanType::AOB) {
            
        }
        else if (scan_type == ScanType::STRING) {
            
        }
        else if (scan_type == ScanType::GROUPED) {
            
        }
    }
    
    void next_scan(string pattern)
    {
        
    }
    
    void reset()
    {
        output.clear();
    }
    
};



bool worldToScreen(const float *matrix, std::vector<float> *from, std::vector<float> *to);

void *GetModuleHandleSafe(const char *);
void *GetProcAddress(void *, const char*);

#endif //HACKME_SDK_HH



































