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
#include "value.hh"


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

/// @ref Andrea Stacchiotti      <andreastacchiotti(a)gmail.com>
/// https://github.com/scanmem/scanmem/commit/9e647cd9384f2e5dc0f14c299ddbd0f89a375721#diff-6b95d7308eeff0c8778ace3e805c8b92
typedef enum __attribute__((__packed__)) {
    flags_empty = 0,
    
    /// Type may be defined
    flag_ui8  = 1 << 0,
    flag_si8  = 1 << 1,
    flag_ui16 = 1 << 2,
    flag_si16 = 1 << 3,
    flag_ui32 = 1 << 4,
    flag_si32 = 1 << 5,
    flag_ui64 = 1 << 6,
    flag_si64 = 1 << 7,
    flag_f32  = 1 << 8,
    flag_f64  = 1 << 9,
    
    /// Or be one of this
    flags_i8  = flag_ui8  | flag_si8,
    flags_i16 = flag_ui16 | flag_si16,
    flags_i32 = flag_ui32 | flag_si32,
    flags_i64 = flag_ui64 | flag_si64,
    
    /// If unknown initial
    flags_full = flags_i8 | flags_i16 | flags_i32 | flags_i64 | flag_f32 | flag_f64,
    
    /// Size
    flags_8   = flags_i8,
    flags_16  = flags_i16,
    flags_32  = flags_i32 | flag_f32,
    flags_64  = flags_i64 | flag_f64,
} Flags;

#define SIZEOF_FLAG(flags) ( \
    (flags) & flags_64?8:    \
    (flags) & flags_32?4:    \
    (flags) & flags_16?2:    \
    (flags) & flags_8 ?1:0)

#define PRINT_VALUE_AS_BYTES(e) {\
    size_t size = SIZEOF_FLAG((e).flags);\
    if      (size == 1) printf("%02x", (e).value.bytes[0]);\
    else if (size == 2) printf("%02x %02x", (e).value.bytes[0], (e).value.bytes[1]);\
    else if (size == 4) printf("%02x %02x %02x %02x", (e).value.bytes[0], (e).value.bytes[1],\
                                                      (e).value.bytes[2], (e).value.bytes[3]);\
    else if (size == 8) printf("%02x %02x %02x %02x %02x %02x %02x %02x", (e).value.bytes[0], (e).value.bytes[1],\
                                                                          (e).value.bytes[2], (e).value.bytes[3],\
                                                                          (e).value.bytes[4], (e).value.bytes[5],\
                                                                          (e).value.bytes[6], (e).value.bytes[7]);\
    else printf("NaN");}


class AddressEntry {
public:
    Flags flags = Flags::flags_empty;
    
    union {
        int8_t   si8;
        uint8_t  ui8;
        int16_t  si16;
        uint16_t ui16;
        int32_t  si32;
        uint32_t ui32;
        int64_t  si64;
        uint64_t ui64;
        float    f32;
        double   f64;
        byte     bytes[sizeof(uint64_t)];
    } value { ui64:0 };
    
    union {
        uintptr_t data;
        byte bytes[sizeof(uintptr_t)];
    } address { data:0 };
    
    const region_t *region = nullptr;
    
    AddressEntry() = default;
    
    AddressEntry(Flags flags, uintptr_t address, const region_t *region = nullptr, byte *value = nullptr)
    {
        this->flags = flags;
        this->address.data = address;
        this->region = region;
        
        if (value)
            for(size_t i = 0; i < SIZEOF_FLAG(flags); i++)
                this->value.bytes[i] = value[i];
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
                      vector<AddressEntry> entries,  //attempt to implement Number scanner
                      size_t increment = 1);        //attempt to implement "Fast scan" (он же так работает?)
};


class HandleScanner {
public:
    Handle *handle;
    ScanType scan_type;
    size_t increment;
    vector<AddressEntry> matches;
    
    explicit HandleScanner(Handle *handle,
                           ScanType scan,
                           size_t increment) 
    {
        this->handle = handle;
        this->scan_type = scan;
        this->increment = increment;
    }
    
    
    void first_scan(const string &string_value, Flags flags)
    {
        printf("~~~ First scan~~~ value: %s\n", string_value.c_str());
        matches.clear();
        
        if (scan_type == ScanType::NUMBER) {
            vector<AddressEntry> values;
            
            AddressEntry entry;
            bool isSigned = false;
            
            if (flags == flags_i64 || flags == flags_full) {
                try {
                    entry.value.si64 = lexical_cast<int64_t>(string_value.c_str(), &isSigned);
                    if (isSigned) {
                        entry.flags = flag_si64;
                        printf("Long: type is signed\n");
                    } else {
                        entry.flags = flags_i64;
                        printf("Long: type is signed or unsigned\n");
                    }
                    values.push_back(entry);
                } catch (lexical_cast_range &) {
                    try {
                        entry.value.ui64 = lexical_cast<uint64_t>(string_value.c_str());
                        entry.flags = Flags::flag_ui64;
                        printf("Long: type is unsigned\n");
                        values.push_back(entry);
                    } catch (lexical_cast_sign &) {
                    } catch (lexical_cast_range &) {
                        printf("Long: value is not integer\n");
                    }
                }
            }
            if (flags == flags_i32 || flags == flags_full) {
                try {
                    entry.value.si32 = lexical_cast<int32_t>(string_value.c_str(), &isSigned);
                    if (isSigned) {
                        entry.flags = flag_si32;
                        printf("Int: type is signed\n");
                    } else {
                        entry.flags = flags_i32;
                        printf("Int: type is signed or unsigned\n");
                    }
                    values.push_back(entry);
                } catch (lexical_cast_range &) {
                    try {
                        entry.value.ui32 = lexical_cast<uint32_t>(string_value.c_str());
                        entry.flags = Flags::flag_ui32;
                        printf("Int: type is unsigned\n");
                        values.push_back(entry);
                    } catch (lexical_cast_sign &) {
                    } catch (lexical_cast_range &) {
                    }
                }
            }
            if (flags == flags_i16 || flags == flags_full) {
                try {
                    entry.value.si16 = lexical_cast<int16_t>(string_value.c_str(), &isSigned);
                    if (isSigned) {
                        entry.flags = flag_si16;
                        printf("Short: type is signed\n");
                    } else {
                        entry.flags = flags_i16;
                        printf("Short: type is signed or unsigned\n");
                    }
                    values.push_back(entry);
                } catch (lexical_cast_range &) {
                    try {
                        entry.value.ui16 = lexical_cast<uint16_t>(string_value.c_str());
                        entry.flags = Flags::flag_ui16;
                        printf("Short: type is unsigned\n");
                        values.push_back(entry);
                    } catch (lexical_cast_sign &) {
                    } catch (lexical_cast_range &) {
                    }
                }
            }
            if (flags == flags_i8 || flags == flags_full) {
                try {
                    entry.value.si8 = lexical_cast<int8_t>(string_value.c_str(), &isSigned);
                    if (isSigned) {
                        entry.flags = flag_si8;
                        printf("Char: type is signed\n");
                    } else {
                        entry.flags = flags_i8;
                        printf("Char: type is signed or unsigned\n");
                    }
                    values.push_back(entry);
                } catch (lexical_cast_range &) {
                    try {
                        entry.value.ui8 = lexical_cast<uint8_t>(string_value.c_str());
                        entry.flags = Flags::flag_ui8;
                        printf("Char: type is unsigned\n");
                        values.push_back(entry);
                    } catch (lexical_cast_sign &) {
                    } catch (lexical_cast_range &) {
                    }
                }
            }
            
            /* boost */
            if (flags == flag_f64 || flags == flags_full) {
                try {
                    entry.value.f64 = boost::lexical_cast<double>(string_value);
                    entry.flags = Flags::flag_f64;
                    values.push_back(entry);
                } catch (boost::bad_lexical_cast &) {
                    printf("Error: value \"%s\" isn't double\n", string_value.c_str());
                }
            }
            if (flags == flag_f32 || flags == flags_full) {
                try {
                    entry.value.f32 = boost::lexical_cast<float>(string_value);
                    entry.flags = Flags::flag_f32;
                    values.push_back(entry);
                } catch (boost::bad_lexical_cast &) {
                    printf("Error: value \"%s\" isn't float\n", string_value.c_str());
                }
            }
            
            /// Scanning
            if (values.empty()) {
                printf("There is no any value\n");
                return;
            }
            
            for(size_t i = 0; i < values.size(); i++) {
                printf("patterns[%li]: ", i);
                PRINT_VALUE_AS_BYTES(values[i])
                printf("\n");
            }
            
            for(size_t i = 0; i < handle->regions.size(); i++) {
                region_t region = handle->regions[i];
                if (region.writable && region.readable) {
                    handle->scan_exact(&matches, &region, values, increment);
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
        matches.clear();
    }
    
};



bool worldToScreen(const float *matrix, std::vector<float> *from, std::vector<float> *to);

void *GetModuleHandleSafe(const char *);
void *GetProcAddress(void *, const char*);

#endif //HACKME_SDK_HH



































