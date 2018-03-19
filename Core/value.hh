//
// Структуры, флаги и прочее
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef RE_VALUE_HH
#define RE_VALUE_HH

#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cerrno>
#include <vector>
#include <string>
#include <iostream>

class region_t {
public:
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
    
    friend std::ostream &operator<<(std::ostream &outputStream, const region_t &p)
    {
        return outputStream<<"region: {"<<"filename: \""<<p.filename<<"\", start: "<<std::hex<<p.start<<", end: "<<p.end<<std::dec<<"}";
    }
};


typedef enum {
    ANYNUMBER,              /* ANYINTEGER or ANYFLOAT */
    ANYINTEGER,             /* INTEGER of whatever width */
    ANYFLOAT,               /* FLOAT of whatever width */
    INTEGER8,
    INTEGER16,
    INTEGER32,
    INTEGER64,
    FLOAT32,
    FLOAT64,
    BYTEARRAY,
    STRING
} scan_data_type_t;

typedef enum {
    MATCHANY,                /* for snapshot */
    /* following: compare with a given value */
    MATCHEQUALTO,
    MATCHNOTEQUALTO,
    MATCHGREATERTHAN,
    MATCHLESSTHAN,
    MATCHRANGE,
    /* following: compare with the old value */
    MATCHUPDATE,
    MATCHNOTCHANGED,
    MATCHCHANGED,
    MATCHINCREASED,
    MATCHDECREASED,
    /* following: compare with both given value and old value */
    MATCHINCREASEDBY,
    MATCHDECREASEDBY
} scan_match_type_t;


/* some routines for working with value_t structures */

/* match_flags: they MUST be implemented as an `uint16_t`, the `__packed__` ensures so.
 * They are reinterpreted as a normal integer when scanning for VLT, which is
 * valid for both endians, as the flags are ordered from smaller to bigger.
 * NAMING: Primitive, single-bit flags are called `flag_*`, while aggregates,
 * defined for convenience, are called `flags_*`*/
typedef enum __attribute__((__packed__)) {
    flags_empty = 0,
    
    flag_u8b  = 1 << 0,  /* could be an unsigned  8-bit variable (e.g. unsigned char)      */
    flag_s8b  = 1 << 1,  /* could be a    signed  8-bit variable (e.g. signed char)        */
    flag_u16b = 1 << 2,  /* could be an unsigned 16-bit variable (e.g. unsigned short)     */
    flag_s16b = 1 << 3,  /* could be a    signed 16-bit variable (e.g. short)              */
    flag_u32b = 1 << 4,  /* could be an unsigned 32-bit variable (e.g. unsigned int)       */
    flag_s32b = 1 << 5,  /* could be a    signed 32-bit variable (e.g. int)                */
    flag_u64b = 1 << 6,  /* could be an unsigned 64-bit variable (e.g. unsigned long long) */
    flag_s64b = 1 << 7,  /* could be a    signed 64-bit variable (e.g. long long)          */
    
    flag_f32b = 1 << 8,  /* could be a 32-bit floating point variable (i.e. float)         */
    flag_f64b = 1 << 9,  /* could be a 64-bit floating point variable (i.e. double)        */
    
    flags_i8b  = flag_u8b  | flag_s8b,
    flags_i16b = flag_u16b | flag_s16b,
    flags_i32b = flag_u32b | flag_s32b,
    flags_i64b = flag_u64b | flag_s64b,
    
    flags_integer = flags_i8b | flags_i16b | flags_i32b | flags_i64b,
    flags_float = flag_f32b | flag_f64b,
    flags_all = flags_integer | flags_float,
    
    flags_8b   = flags_i8b,
    flags_16b  = flags_i16b,
    flags_32b  = flags_i32b | flag_f32b,
    flags_64b  = flags_i64b | flag_f64b,
    
    flags_max = 0xffffU /* ensures we're using an uint16_t */
} match_flags;


/* Possible flags per scan data type: if an incoming uservalue has none of the
 * listed flags we're sure it's not going to be matched by the scan,
 * so we reject it without even trying */
static match_flags scan_data_type_to_flags[] = {
        [ANYNUMBER]  = flags_all,
        [ANYINTEGER] = flags_integer,
        [ANYFLOAT]   = flags_float,
        [INTEGER8]   = flags_i8b,
        [INTEGER16]  = flags_i16b,
        [INTEGER32]  = flags_i32b,
        [INTEGER64]  = flags_i64b,
        [FLOAT32]    = flag_f32b,
        [FLOAT64]    = flag_f64b,
        [BYTEARRAY]  = flags_max,
        [STRING]     = flags_max
};

static inline size_t flags_to_memlength(scan_data_type_t scan_data_type, match_flags flags)
{
    switch(scan_data_type) {
        case BYTEARRAY:
        case STRING:
            return flags;
        default: /* NUMBER */
            return (flags & flags_64b)?8:
                   (flags & flags_32b)?4:
                   (flags & flags_16b)?2:
                   (flags & flags_8b )?1:0;
    }
}

/* This union describes 8 bytes retrieved from target memory.
 * Pointers to this union are the only ones that are allowed to be unaligned:
 * to avoid performance degradation/crashes on arches that don't support unaligned access
 * (e.g. ARM) we access unaligned memory only through the attributes of this packed union.
 * As described in http://www.alfonsobeato.net/arm/how-to-access-safely-unaligned-data/ ,
 * a packed structure forces the compiler to write general access methods to its members
 * that don't depend on alignment.
 * So NEVER EVER dereference a mem64_t*, but use its accessors to obtain the needed type.
 */
typedef union __attribute__((packed)) {
    int8_t int8_value;
    uint8_t uint8_value;
    int16_t int16_value;
    uint16_t uint16_value;
    int32_t int32_value;
    uint32_t uint32_value;
    int64_t int64_value;
    uint64_t uint64_value;
    float float32_value;
    double float64_value;
    uint8_t bytes[sizeof(int64_t)];
    char chars[sizeof(int64_t)];
} mem64_t;

/* bytearray wildcards: they must be uint8_t. They are ANDed with the incoming
 * memory before the comparison, so that '??' wildcards always return true
 * It's possible to extend them to fully granular wildcard-ing, if needed */
typedef enum __attribute__ ((__packed__)) {
    FIXED = 0xffu,
    WILDCARD = 0x00u,
} wildcard_t;

/* this struct describes values provided by users */
typedef struct {
    int8_t int8_value;
    uint8_t uint8_value;
    int16_t int16_value;
    uint16_t uint16_value;
    int32_t int32_value;
    uint32_t uint32_value;
    int64_t int64_value;
    uint64_t uint64_value;
    float float32_value;
    double float64_value;
    
    std::vector<uint8_t> bytearray_value;
    std::vector<wildcard_t> wildcard_value;
    
    const char *string_value;
    const wchar_t *wstring_value;
    
    match_flags flags;
} uservalue_t;

bool parse_uservalue_int(const char *nptr, uservalue_t *val);
bool parse_uservalue_float(const char *nptr, uservalue_t *val);
bool parse_uservalue_number(const char *nptr, uservalue_t *val);     // parse int or float
bool parse_uservalue_bytearray(const char *text, uservalue_t *val);
bool parse_uservalue_string(const char *text, uservalue_t *val);

#endif //RE_VALUE_HH

#pragma clang diagnostic pop