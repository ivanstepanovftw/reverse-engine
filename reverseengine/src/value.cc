/*
    This file is part of Reverse Engine.

    

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>
    Copyright (C) 2017      Andrea Stacchiotti <andreastacchiotti@gmail.com>

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <reverseengine/value.hh>

size_t RE::flags_to_memlength(Edata_type scan_data_type, uint16_t flags)
{
    switch (scan_data_type) {
        case Edata_type::BYTEARRAY:
        case Edata_type::STRING:
            return flags;
        default: /* NUMBER */
            return (flags & flags_64b) ? 8 :
                   (flags & flags_32b) ? 4 :
                   (flags & flags_16b) ? 2 :
                   (flags & flags_8b) ? 1 : 0;
    }
}


size_t RE::flags_to_type(Edata_type scan_data_type, uint16_t flags)
{
    switch (scan_data_type) {
        case Edata_type::BYTEARRAY:
        case Edata_type::STRING:
            return flags;
        default: /* NUMBER */
            return (flags & flags_64b) ? 8 :
                   (flags & flags_32b) ? 4 :
                   (flags & flags_16b) ? 2 :
                   (flags & flags_8b) ? 1 : 0;
    }
}

/// https://github.com/scanmem/scanmem/blob/master/sets.c
size_t RE::parse_uservalue_int(const std::string& text, RE::Cuservalue *val)
{
    const char *text_c = text.c_str();
    char *endptr;
    char *endptr2;
    
    /// parse it as signed int
    errno = 0;
    int64_t snum = strtoll(text_c, &endptr, 0);
    bool valid_sint = (errno == 0) && (*endptr == '\0');
    
    /// parse it as unsigned int
    errno = 0;
    uint64_t unum = strtoull(text_c, &endptr2, 0);
    bool valid_uint = (text[0] != '-') && (errno == 0) && (*endptr2 == '\0');
    
    if (!valid_sint && !valid_uint)
        return strlen(text_c) - MAX(strlen(endptr), strlen(endptr2));
    
    /// determine correct flags
    if (valid_sint && snum >= INT64_MIN && snum <=  INT64_MAX) { val->i64 = static_cast< int64_t>(snum); val->flags |= RE::flag_i64; }
    if (valid_sint && snum >= INT32_MIN && snum <=  INT32_MAX) { val->i32 = static_cast< int32_t>(snum); val->flags |= RE::flag_i32; }
    if (valid_sint && snum >= INT16_MIN && snum <=  INT16_MAX) { val->i16 = static_cast< int16_t>(snum); val->flags |= RE::flag_i16; }
    if (valid_sint && snum >=  INT8_MIN && snum <=   INT8_MAX) { val->i8  = static_cast< int8_t >(snum); val->flags |= RE::flag_i8 ; }
    if (valid_uint &&                      unum <= UINT64_MAX) { val->u64 = static_cast<uint64_t>(unum); val->flags |= RE::flag_u64; }
    if (valid_uint &&                      unum <= UINT32_MAX) { val->u32 = static_cast<uint32_t>(unum); val->flags |= RE::flag_u32; }
    if (valid_uint &&                      unum <= UINT16_MAX) { val->u16 = static_cast<uint16_t>(unum); val->flags |= RE::flag_u16; }
    if (valid_uint &&                      unum <=  UINT8_MAX) { val->u8  = static_cast<uint8_t >(unum); val->flags |= RE::flag_u8 ; }
    
    /// If signed flag exist and unsigned not exist, fill unsigned value. Used at first_scan() to scan 5% faster.
    if ((val->flags & RE::flag_i64) && !(val->flags & RE::flag_u64)) { val->u64 = static_cast<uint64_t>(val->i64); }
    if ((val->flags & RE::flag_i32) && !(val->flags & RE::flag_u32)) { val->u32 = static_cast<uint32_t>(val->i32); }
    if ((val->flags & RE::flag_i16) && !(val->flags & RE::flag_u16)) { val->u16 = static_cast<uint16_t>(val->i16); }
    if ((val->flags & RE::flag_i8 ) && !(val->flags & RE::flag_u8 )) { val->u8  = static_cast<uint8_t >(val->i8 ); }
    return strlen(text_c);
}

//fixme this shit parses float only with comma, dot won't parse
//fixme also can't parse "nan", "inf" and "-inf"
//    double a = -1./0.;
//    double b = 0./0. ;
//    double c = 1./0. ;
//    clog<<"-1./0.  = "<<a<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&a)<<dec<<endl;
//    clog<<"0./0.   = "<<b<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&b)<<dec<<endl;
//    clog<<"1./0.   = "<<c<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&c)<<dec<<endl;
size_t RE::parse_uservalue_float(const std::string& text, RE::Cuservalue *val)
{
    char *endptr;
    const char *text_c = text.c_str();
    
    errno = 0;
    double num = strtod(text_c, &endptr);
    if ((errno != 0) || (*endptr != '\0'))
        return strlen(text_c) - strlen(endptr);
    
    val->flags |= RE::flags_float;
    val->f32 = static_cast<float>(num);
    val->f64 = num;
    return strlen(text_c);
}

size_t RE::parse_uservalue_number(const std::string& text, RE::Cuservalue *val)
{
    using namespace std;
    size_t ret;
    if ((ret = parse_uservalue_int(text, val)) && ret == text.size()) {
        val->flags |= RE::flags_float;
        if (val->flags & RE::flag_i64) {
            val->f32 = static_cast<float>(val->i64);
            val->f64 = static_cast<double>(val->i64);
        } else {
            val->f32 = static_cast<float>(val->u64);
            val->f64 = static_cast<double>(val->u64);
        }
        return ret;
    } else if ((ret = parse_uservalue_float(text, val)) && ret == text.size()) {
        double num = val->f64;
        if (num >= INT64_MIN && num <=  INT64_MAX) { val->i64 = static_cast< int64_t>(num); val->flags |= flag_i64; }
        if (num >= INT32_MIN && num <=  INT32_MAX) { val->i32 = static_cast< int32_t>(num); val->flags |= flag_i32; }
        if (num >= INT16_MIN && num <=  INT16_MAX) { val->i16 = static_cast< int16_t>(num); val->flags |= flag_i16; }
        if (num >=  INT8_MIN && num <=   INT8_MAX) { val->i8  = static_cast< int8_t >(num); val->flags |= flag_i8 ; }
        if (num >=         0 && num <= UINT64_MAX) { val->u64 = static_cast<uint64_t>(num); val->flags |= flag_u64; }
        if (num >=         0 && num <= UINT32_MAX) { val->u32 = static_cast<uint32_t>(num); val->flags |= flag_u32; }
        if (num >=         0 && num <= UINT16_MAX) { val->u16 = static_cast<uint16_t>(num); val->flags |= flag_u16; }
        if (num >=         0 && num <=  UINT8_MAX) { val->u8  = static_cast<uint8_t >(num); val->flags |= flag_u8 ; }
        
        /// If signed flag exist and unsigned not exist, fill unsigned value. Used at first_scan() to scan 5% faster.
        if ((val->flags & flag_i64) && !(val->flags & flag_u64)) { val->u64 = static_cast<uint64_t>(val->i64); }
        if ((val->flags & flag_i32) && !(val->flags & flag_u32)) { val->u32 = static_cast<uint32_t>(val->i32); }
        if ((val->flags & flag_i16) && !(val->flags & flag_u16)) { val->u16 = static_cast<uint16_t>(val->i16); }
        if ((val->flags & flag_i8 ) && !(val->flags & flag_u8 )) { val->u8  = static_cast<uint8_t >(val->i8 ); }
        return ret;
    }
    clog<<"parse_uservalue_number(): cannot parse ret: ret: "<<ret<<endl;
    return ret;
}

/* parse bytearray, it will allocate the arrays itself, then needs to be free'd by `free_uservalue()` */
//TODO create entry for mask, not only for pattern
//FIXME UNDONE
size_t RE::parse_uservalue_bytearray(const std::string& text_s, RE::Cuservalue *val)
{
    using namespace std;
    vector<uint8_t> bytes_array;
    vector<RE::wildcard_t> wildcards_array;
    
    /// @see https://stackoverflow.com/questions/7397768/choice-between-vectorresize-and-vectorreserve
    // fixme[low]: hardcode
    bytes_array.reserve(32);
    wildcards_array.reserve(32);
    
    /// skip past any whitespace
    char *text = const_cast<char *>(&text_s[0]);
    while (isspace(*text))
        ++text;
    
    /// split text by whitespace and(or) \x-like sequence "\xDE\xAD" (even "x\DExAD", sorry xD) and "BE EF" 
    char *cur_str;
    cur_str = strtok(const_cast<char *>(text), R"( \x)");
    
    for(; *cur_str != '\0';) { //fixme должно же работать, не?
        /// test its length
        if (strlen(cur_str) != 2)
            return false;
        
        /// if unknown value
        if ((strcmp(cur_str, "??") == 0)) {
            bytes_array.push_back(0x00);
            wildcards_array.push_back(WILDCARD);
        } else {
            /// parse as hex integer
            char *endptr;
            uint8_t cur_byte = static_cast<uint8_t>(strtoul(cur_str, &endptr, 16));
            if (*endptr != '\0')
                return false;
            
            bytes_array.push_back(cur_byte);
            wildcards_array.push_back(FIXED);
        }
        
        /// get next byte
        cur_str = strtok(NULL, R"( \x)");
    }
    
    /* everything is ok */
    val->wildcard_value = wildcards_array.data();
    val->bytearray_value = bytes_array.data();
    val->flags = flags_all;
    return true;
}

//FIXME[low]: to revision
size_t RE::parse_uservalue_string(const std::string& text_s, RE::Cuservalue *val)
{
    using namespace std;
    vector<uint8_t> bytes_array;
    vector<RE::wildcard_t> wildcards_array;
    
    /// @see https://stackoverflow.com/questions/7397768/choice-between-vectorresize-and-vectorreserve
    // fixme[low]: hardcode
    bytes_array.reserve(32);
    wildcards_array.reserve(32);
    
    /// skip past any whitespace
    char *text = const_cast<char *>(&text_s[0]);
    while (isspace(*text))
        ++text;
    
    /// split text by whitespace and(or) \x-like sequence "\xDE\xAD" and "BE EF" (and "x\DExAD", sorry xD)
    char *cur_str;
    cur_str = strtok(const_cast<char *>(text), R"( \x)");
    
    for(; *cur_str != '\0';) { //fixme должно же работать, не?
        /// test its length
        if (strlen(cur_str) != 2)
            return false;
        
        /// if unknown value
        if ((strcmp(cur_str, "??") == 0)) {
            bytes_array.push_back(0x00);
            wildcards_array.push_back(RE::WILDCARD);
        } else {
            /// parse as hex integer
            char *endptr;
            uint8_t cur_byte = static_cast<uint8_t>(strtoul(cur_str, &endptr, 16));
            if (*endptr != '\0')
                return false;
            
            bytes_array.push_back(cur_byte);
            wildcards_array.push_back(RE::FIXED);
        }
        
        /// get next byte
        cur_str = strtok(NULL, R"( \x)");
    }
    
    /* everything is ok */
    val->wildcard_value = wildcards_array.data();
    val->bytearray_value = bytes_array.data();
    val->flags = RE::flags_all;
    return true;
}
