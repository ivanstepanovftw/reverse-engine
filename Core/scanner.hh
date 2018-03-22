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

class match {
public:
    const region_t *region;
    uintptr_t offset;
    union {
        uintptr_t data;
        uint8_t bytes[sizeof(uintptr_t)];
    } address { data:0 };
    mem64_t val;
    mem64_t val_old;
    match_flags flags;
    
    explicit match(const region_t *region, uintptr_t offset, mem64_t memory_ptr, match_flags userflag, mem64_t *val_old = nullptr) {
        this->region = region;
        this->offset = offset;
        this->address.data = region->start + offset;
        this->val = memory_ptr;
        this->flags = userflag;
        if (val_old)
            this->val_old = *val_old;
    }
};


class Scanner
{
public:
    Handle *handle;
    std::vector<match> matches;
    bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
        
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    //обрабатываем текст с текущими параметрами поиска
    bool parse(scan_data_type_t data_type, const char *ustr,
               scan_match_type_t *match_type, uservalue_t *vals);
    
    bool first_scan(scan_data_type_t data_type, const char *ustr);
    
    bool next_scan(scan_data_type_t data_type, const char *ustr);
    
//    bool update();
    
//    bool sm_choose_scanroutine(scan_data_type_t dt, scan_match_type_t mt, const uservalue_t *uval, bool reverse_endianness);
    
    /* This is the function that handles when you enter a value (or >, <, =) for the second or later time (i.e. when there's already a list of matches);
     * it reduces the list to those that still match. It returns false on failure to attach, detach, or reallocate memory, otherwise true. */
    //    bool sm_checkmatches(scan_match_type_t match_type, const uservalue_t *uservalue);
    
    
    /// Iterator over target memory
private:
    region_t *i_region;
    size_t i_region_number = 0;
    uintptr_t i_totalsize = 0;
    uint8_t *i_buffer = nullptr;
    uintptr_t i_offset = 0;
    mem64_t *i_memory;
    
    bool i_nextRegion() {
        for(; i_region_number < handle->regions.size();) {
            if (stop_flag) return false;
            i_region = &handle->regions[i_region_number++];
            if (!i_region->writable || !i_region->readable)
                continue;
            /// calculate size of region
            i_totalsize = i_region->end - i_region->start;
            /// allocate buffer
            delete [] i_buffer;
            i_buffer = new uint8_t[i_totalsize];
            /// read into buffer
            if (!handle->read(i_buffer, (i_region->start), i_totalsize)) {
                std::clog<<"error: invalid region: cant read memory: "<<i_region<<std::endl;
                continue;
            }
            return true;
        }
        /// reset for next execution
        i_region_number = 0;
        /// no region found
        return false;
    }
};

#endif //RE_SCANNER_HH

//#pragma clang diagnostic pop





















