#ifndef RE_CORE_HH
#define RE_CORE_HH

#include <sys/uio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include "value.hh"
#include "external.hh"

//class Entry {
//public:
//    Flags flags = flags_empty;
//    
//    union ByteType {
//        uint64_t u64;
//        int64_t  s64;
//        uint32_t u32;
//        int32_t  s32;
//        uint16_t u16;
//        int16_t  s16;
//        uint8_t  u8;
//        int8_t   s8;
//        double   f64;
//        float    f32;
//        byte     bytes[sizeof(int64_t)];
//    } value { u64:0 };
//    
//    union {
//        uintptr_t data;
//        byte bytes[sizeof(uintptr_t)];
//    } address { data:0 };
//    
//    const region_t *region;
//    
//    Entry() = default;
//    
//    Entry(Flags flags, uintptr_t address, const region_t *region = nullptr, byte *value = nullptr)
//    {
//        this->flags = flags;
//        this->address.data = address;
//        this->region = region;
//        
//        //fixme value: 21245, double: 00 00 00 00 40 bf d4 40, float: 00 00 00 00
//        if (value)
//            for(size_t i = 0; i < SIZEOF_FLAG(flags); i++)
//                this->value.bytes[i] = value[i];
//    }
//    
//    Entry(Flags flags, byte *value = nullptr)
//    {
//        this->flags = flags;
//        this->region = region;
//        
//        //fixme value: 21245, double: 00 00 00 00 40 bf d4 40, float: 00 00 00 00
//        if (value)
//            for(size_t i = 0; i < SIZEOF_FLAG(flags); i++)
//                this->value.bytes[i] = value[i];
//    }
//    
//    Entry(const Entry &e)
//    {
//        this->flags = e.flags;
//        this->address.data = e.address.data;
//        this->region = e.region;
//        
//        //fixme value: 21245, double: 00 00 00 00 40 bf d4 40, float: 00 00 00 00
//        for(size_t i = 0; i < SIZEOF_FLAG(flags); i++)
//            this->value.bytes[i] = e.value.bytes[i];
//    }
//};

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
    bool read(void *out, uintptr_t address, size_t size);

    bool write(uintptr_t address, void *buffer, size_t size);

    // Modules
    void updateRegions();

    region_t *getRegion(const std::string &region_name = "");

    region_t *getRegionOfAddress(uintptr_t address);

    bool getCallAddress(uintptr_t *out, uintptr_t address);
    
    static bool dumpMemory(const std::string &path) {
        //todo
        return false;
    }
    
    //    bool findPointer(void *out, uintptr_t address, vector<uintptr_t> offsets, size_t size, 
//                     size_t point_size = sizeof(uintptr_t), ostream *ss = nullptr);
//
//
//    bool findPattern(uintptr_t *out, region_t *region, const char *pattern, const char *mask);
//    
//    size_t findPattern(vector<uintptr_t> *out, region_t *region, const char *pattern, const char *mask);
//    
//    size_t scan_exact(vector<Entry> *out,
//                      const region_t *region,
//                      vector<Entry> entries,  //attempt to implement Number scanner
//                      size_t increment = 1);  //attempt to implement "Fast scan" (он же так работает?)
};

#endif //RE_CORE_HH



































