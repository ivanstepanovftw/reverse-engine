//
// Created by root on 09.03.18.
//

#ifndef RE_SCANNER_HH
#define RE_SCANNER_HH

#include "core.hh"
#include "inttypes.h"
#include "value.hh"

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
    match_flags userflag;
    
    explicit match(const region_t *region, uintptr_t offset, mem64_t memory_ptr, match_flags userflag) {
        this->region = region;
        this->offset = offset;
        this->address.data = region->start + offset;
        this->val = memory_ptr;
        this->userflag = userflag;
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
    bool parse(scan_data_type_t data_type, const char *ustr, scan_match_type_t *mt, uservalue_t *vals);
    
    bool first_scan(scan_data_type_t data_type, const char *ustr);
    
//    bool next_scan(scan_data_type_t data_type, char *ustr);
    
//    bool update();
    
//    bool sm_choose_scanroutine(scan_data_type_t dt, scan_match_type_t mt, const uservalue_t *uval, bool reverse_endianness);
    
    /* This is the function that handles when you enter a value (or >, <, =) for the second or later time (i.e. when there's already a list of matches);
     * it reduces the list to those that still match. It returns false on failure to attach, detach, or reallocate memory, otherwise true. */
    //    bool sm_checkmatches(scan_match_type_t match_type, const uservalue_t *uservalue);
    
};

#endif //RE_SCANNER_HH
