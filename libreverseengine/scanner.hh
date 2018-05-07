/*
    This file is part of Reverse Engine.

    Array of scanner results.

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>
    Copyright (C) 2017      Andrea Stacchiotti <andreastacchiotti@gmail.com>
    Copyright (C) 2016-2017 Sebastian Parschauer <s.parschauer@gmx.de>

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

#ifndef RE_SCANNER_HH
#define RE_SCANNER_HH

#include <cinttypes>
#include <memory>
#include <cmath>
#include <chrono>
#include <fcntl.h>
#include <sys/mman.h>
#include <chrono>
#include <deque>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>
#include "core.hh"
#include "value.hh"

namespace bio = boost::iostreams;


class match_t {
public:
    uintptr_t address;
    mem64_t memory;
    mem64_t memory_old;
    uint16_t flags;
    
    match_t() {
        this->address = 0;
        this->flags = 0;
    }
    
    explicit match_t(uintptr_t address, mem64_t memory, uint16_t userflag = flags_empty) {
        this->address = address;
        this->memory = memory;
        this->flags = userflag;
    }
};


/* Single match struct */
struct byte_with_flag /// :genius:
{
    uint8_t old_value;
    uint16_t flags;
};

class swath_t
{
public:
    uintptr_t base_address;
    size_t number_of_bytes;
    std::vector<byte_with_flag> data;
    
    size_t
    index_of_last_element()
    {
        return number_of_bytes - 1;
    }
    
    uintptr_t
    remote_address_of_nth_element(size_t n)
    {
        return base_address + n;
    }
    
    uintptr_t
    remote_address_of_last_element()
    {
        return (remote_address_of_nth_element(index_of_last_element()));
    }
    
    uintptr_t///////////////////fixme possibly wrong
    local_address_beyond_nth_element(size_t n)
    {
        return reinterpret_cast<uintptr_t>(&data[n + 1]);
    }
    
    uintptr_t///////////////////////todo rename to next
    local_address_beyond_last_element()
    {
        return local_address_beyond_nth_element(index_of_last_element());
    }
};



/* Location of a match in a matches_t */
struct match_location
{
    swath_t *swath;
    size_t index;
};


class matches_t {
public:
    size_t max_needed_bytes;
    size_t bytes_allocated;
    swath_t *swaths;
    
    size_t matches_size;
    swath_t *swath_last; //current writing swath
    
    bool////////////////////////////////////////////////////////
    allocate_enough_to_reach(char *last_byte_to_reach_plus_one,
                             swath_t **swath_pointer_to_correct) //fixme aliasing
    {
        using namespace std;
        size_t bytes_needed = last_byte_to_reach_plus_one - (char *) swaths;
        //fixme if need to
        clog<<"bytes_needed: "<<bytes_needed<<endl;
        
        if (bytes_needed <= this->bytes_allocated)
            return true;
        
        /* allocate twice as much each time,
           so we don't have to do it too often */
        size_t bytes_to_allocate = this->bytes_allocated;
        while (bytes_to_allocate < bytes_needed)
            bytes_to_allocate *= 2;
        
        printf("to_allocate %ld, max %ld\n", 
               bytes_to_allocate,
               this->max_needed_bytes);
        
        /* sometimes we know an absolute max that we will need */
        if (this->max_needed_bytes < bytes_to_allocate) {
            assert(this->max_needed_bytes >= bytes_needed);
            bytes_to_allocate = this->max_needed_bytes;
        }
        
        size_t delta = (char *) (*swath_pointer_to_correct) - (char *) swaths;
        
        if ((swaths = (swath_t *)(realloc(swaths, bytes_to_allocate))) == NULL)
            return false;
    
        this->bytes_allocated = bytes_to_allocate;
        
        /* Put the swath pointer back where it should be, if needed.
           We cast everything to void pointers in this line to make
           sure the math works out. */
        if (swath_pointer_to_correct) {
            *swath_pointer_to_correct = (swath_t *) ((char *)(*swath_pointer_to_correct) + delta);
        }
        
        return true;
    }///////////////////////////////////////////////////////
    
    
    /* returns a pointer to the swath to which the element was added -
       i.e. the last swath in the array after the operation */
    void//////////////////
    add_element(match_t m)
    {
        uintptr_t remote_address = m.address;
        uint8_t new_byte = m.memory.u8;
        uint16_t new_flags = m.flags;
        
        if (swath_last->number_of_bytes == 0) {
            assert(swath_last->base_address == 0);
            
            /* we have to overwrite this as a new swath */
            this->allocate_enough_to_reach((char *) swath_last
                                           + sizeof(swath_t)
                                           + sizeof(byte_with_flag),
                                           &swath_last);
            
            swath_last->base_address = remote_address;
        }
        else {
            size_t local_index_excess =
                    remote_address - swath_last->remote_address_of_last_element();
            
            size_t local_address_excess =
                    local_index_excess * sizeof(byte_with_flag);
            
            size_t needed_size_for_a_new_swath =
                    sizeof(swath_t);
            
            if (local_address_excess >= needed_size_for_a_new_swath) {
                /* It is more memory-efficient to start a new swath.
                 * The equal case is decided for a new swath, so that
                 * later we don't needlessly iterate through a bunch
                 * of empty values */
                this->allocate_enough_to_reach(reinterpret_cast<char *>(swath_last->local_address_beyond_last_element()
                                                                        + needed_size_for_a_new_swath),
                                               &swath_last);
                
                swath_last = (swath_t *)(swath_last->local_address_beyond_last_element()); //fixme !!!!!!!!!!!!!!!!!!!!!
                swath_last->base_address = remote_address;
                swath_last->number_of_bytes = 0;
            }
            else {
                /* It is more memory-efficient to write over the intervening
                   space with null values */
                this->allocate_enough_to_reach(reinterpret_cast<char *>(swath_last->local_address_beyond_last_element()
                                                                        + local_address_excess),
                                               &swath_last);
                
                switch (local_index_excess) {
                    case 1:
                        /* do nothing, the new value is right after the old */
                        break;
                    case 2:
                        memset(reinterpret_cast<void *>(swath_last->local_address_beyond_last_element()),
                               0,
                               sizeof(byte_with_flag));
                        break;
                    default:
                        /* slow due to unknown size to be zeroed */
                        memset(reinterpret_cast<void *>(swath_last->local_address_beyond_last_element()),
                               0,
                               local_address_excess - sizeof(byte_with_flag));
                        break;
                }
                swath_last->number_of_bytes += local_index_excess - 1;
            }
        }
        
        /* add me */
        byte_with_flag *dataptr = (byte_with_flag *)(swath_last->local_address_beyond_last_element());
        dataptr->old_value = new_byte;
        dataptr->flags = new_flags;
        swath_last->number_of_bytes++;
        matches_size++;
    }
    
    
    bool////////////////////////////
    allocate_array(size_t max_bytes)
    {
        if ((swaths = (swath_t *)(realloc(swaths, sizeof(swath_t)))) == NULL) 
            return false;
        
        /* make enough space for the matches header and a null first swath */
        this->bytes_allocated = sizeof(swath_t);
        this->max_needed_bytes = max_bytes;
        return true;
    }///////////////////////////
    
    
    bool////////////
    null_terminate()
    {
        if (swath_last->number_of_bytes == 0) {
            assert(swath_last->base_address == 0);
        } else {
            swath_last = reinterpret_cast<swath_t *>(swath_last->local_address_beyond_last_element());
            //fixme uglycode below
            this->allocate_enough_to_reach((char *) (swath_last + 1),
                                           &swath_last);
            swath_last->base_address = 0;
            swath_last->number_of_bytes = 0;
        }
    
        size_t bytes_needed = (char *) swath_last
                              + sizeof(swath_t)
                              - ((char *) (this) + sizeof(size_t) + sizeof(size_t) + sizeof(swath_t *));
        // fixme upper !!!!!!!!!!!
        
        if (bytes_needed < this->bytes_allocated) {
            /* reduce matches to its final size */
            if ((swaths = (swath_t *) (realloc(swaths, bytes_needed))) == NULL)
                return false;
    
            this->bytes_allocated = bytes_needed;
        }
        
        return true;
    }///////////
    
    
    match_location///// possibly ok
    nth_match(size_t n)
    {
        size_t i = 0;
        swath_t *reading_swath_index;
        size_t reading_iterator = 0;
        
        reading_swath_index = this->swaths;
        
        while (reading_swath_index->base_address) {
            /* only actual matches are considered */
            if (reading_swath_index->data[reading_iterator].flags != flags_empty) {
                if (i == n)
                    return (match_location) { reading_swath_index, reading_iterator };
                i++;
            }
            
            /* go on to the next one... */
            reading_iterator++;
            if (reading_iterator >= reading_swath_index->number_of_bytes) {
                reading_swath_index = (swath_t *)
                        (reading_swath_index->local_address_beyond_last_element());
                
                reading_iterator = 0;
            }
        }
        
        /* I guess this is not a valid match-id */
        return (match_location) { 0, 0 };
    }//////////////
    
    
    
    /* deletes matches in [start, end) and resizes the matches array */
    //todo resolve this
//    void
//    delete_in_address_range(unsigned long *num_matches,
//                            uintptr_t start_address,
//                            uintptr_t end_address)
//    {
//        size_t reading_iterator = 0;
//        swath_t *reading_swath_index = this->swaths;
//        
//        swath_t reading_swath = *reading_swath_index;
//        
//        swath_t *writing_swath_index = this->swaths;
//        
//        writing_swath_index->base_address = 0;
//        writing_swath_index->number_of_bytes = 0;
//        
//        *num_matches = 0;
//        
//        while (reading_swath.base_address) {
//            uintptr_t address = reading_swath.base_address + reading_iterator;
//            
//            if (address < start_address || address >= end_address) {
//                byte_with_flag old_byte;
//                
//                old_byte = reading_swath_index->data[reading_iterator];
//                
//                /* Still a candidate. Write data.
//                    (We can get away with overwriting in the same array because
//                     it is guaranteed to take up the same number of bytes or fewer,
//                     and because we copied out the reading swath metadata already.)
//                    (We can get away with assuming that the pointers will stay
//                     valid, because as we never add more data to the array than
//                     there was before, it will not reallocate.) */
////                writing_swath_index = this->add_element(writing_swath_index, //TODO 12312312
////                                                        address,
////                                                        old_byte.old_value,
////                                                        old_byte.flags);
//                
//                /* actual matches are recorded */
//                if (old_byte.flags != flags_empty)
//                    (*num_matches)++;
//            }
//            
//            /* go on to the next one... */
//            reading_iterator++;
//            if (reading_iterator >= reading_swath.number_of_bytes) {
//                
//                reading_swath_index = (swath_t *)
//                        (&reading_swath_index->data[reading_swath.number_of_bytes]);
//                
//                reading_swath = *reading_swath_index;
//                
//                reading_iterator = 0;
//            }
//        }
//        
//        this->null_terminate(writing_swath_index);
//    }
    
    
    
    std::vector<region_t> regions;
    std::string           path;
    bio::mapped_file      snapshot_mf;
    bio::mapped_file      flags_mf;
    
    matches_t () {};
    
    explicit matches_t(const bio::mapped_file& snapshot_mapped_file,
                       const bio::mapped_file& flags_mapped_file)
    {
        snapshot_mf = snapshot_mapped_file;
        flags_mf = flags_mapped_file;
    
        char *snapshot_begin = snapshot_mf.data();
        char *snapshot_end = snapshot_begin + snapshot_mf.size();
        char *snapshot = snapshot_begin;
        
        region_t region;
        while(snapshot < snapshot_end) {
            memcpy(&region,
                   snapshot,
                   2*sizeof(region.address));
            if (region.size <= 1) break;
            snapshot += region.size;
            regions.emplace_back(region);
        }
    }
    
    bool operator !() const {
        return !snapshot_mf || !flags_mf;
    }
    
    size_t size() {
        if (!*this) {
            char *flags_begin = flags_mf.data();
            char *flags_end = flags_begin + flags_mf.size();
            char *flags = flags_begin;
    
            size_t size = 0;
            uint16_t flag;
    
            while (flags < flags_end) {
                memcpy(&flag,
                       flags,
                       sizeof(uint16_t));
                if (flag)
                    size++;
                flags += sizeof(uint16_t);
            }
    
            return size;
        } else {
            
        }
    }
};



class bad_uservalue_cast
{
    std::string _text;   // оригинальная строка
    size_t _at;     // указывает, на каком символе ошибка

public:
    bad_uservalue_cast(const std::string &original, const size_t &at) _GLIBCXX_USE_NOEXCEPT {
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
    
    size_t at() const _GLIBCXX_USE_NOEXCEPT {
        return _at;
    };
};



class Scanner
{
public:
    /// Create file for storing matches
    Handle *handle;
    bool stop_flag = false;
    double scan_progress = 0;
    uintptr_t step = 1;
    
    explicit Scanner(Handle *handle) {
        this->handle = handle;
    }
    
    void string_to_uservalue(const scan_data_type_t &data_type,
                             const std::string &text,
                             scan_match_type_t *match_type,
                             uservalue_t *vals);
    
    bio::mapped_file make_snapshot(const std::string &path);
    
    bool scan(matches_t& matches_sink,
              const scan_data_type_t& data_type,
              const uservalue_t *uservalue,
              const scan_match_type_t& match_type);
    
    bool flags_compare(const matches_t& matches_source,
                       const matches_t& matches_sink);
    
    bool scan_reset();
    
};

#endif //RE_SCANNER_HH
