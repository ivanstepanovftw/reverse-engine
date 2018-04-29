//
// Created by root on 16.02.18.
//

#include "core.hh"

using namespace std;


//bool //todo короче можно вместо стрима закинуть, например, вектор со стрингами
//Handle::findPointer(void *out, uintptr_t address, std::vector<uintptr_t> offsets, size_t size, size_t point_size, std::ostream *ss) 
//{
//    void *buffer;
//    if (ss) *ss<<"["<<offsets[0]<<"] -> ";
//    if (!this->read(&buffer, (void *)(address), point_size)) return false;
//    if (ss) *ss<<""<<buffer<<std::endl;
//    for(uint64_t i = 0; i < offsets.size()-1; i++) {
//        if (ss) *ss<<"["<<buffer<<" + "<<offsets[i]<<"] -> ";
//        if (!this->read(&buffer, buffer + offsets[i], point_size)) return false;
//        if (ss) *ss<<""<<buffer<<std::endl;
//    }
//    if (ss) *ss<<buffer<<" + "<<offsets[offsets.size()-1]<<" = "<<buffer+offsets[offsets.size()-1]<<" -> ";
//    if (!this->read(out, buffer + offsets[offsets.size() - 1], size)) return false;
//    
//    return true;
//}
//
//bool
//Handle::findPattern(uintptr_t *out, region_t *region, const char *pattern, const char *mask) 
//{
//    char buffer[0x1000];
//    
//    size_t len = strlen(mask);
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->address;
//    size_t chunknum = 0;
//    
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        size_t readaddr = region->address + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {
//            for(size_t b = 0; b < readsize; b++) {
//                size_t matches = 0;
//                
//                // если данные совпадают или пропустить
//                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
//                    matches++;
//                    
//                    if (matches == len) {
////                            printf("Debug Output:\data");
////                            for (int i = 0; i < readsize-b; i++) {
////                                if (i != 0 && i % 8 == 0) {
////                                    printf("\data");
////                                }
////                                printf("%02x ",(unsigned char)buffer[b+i]);
////                            }
////                            printf("\data");
//                        *out = (uintptr_t) (readaddr + b);
//                        return true;
//                    }
//                }
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return false;
//}
//
//size_t
//Handle::findPattern(vector<uintptr_t> *out, region_t *region, const char *pattern, const char *mask) 
//{
//    char buffer[0x1000];
//    
//    size_t len = strlen(mask);
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->address;
//    size_t chunknum = 0;
//    size_t found = 0;
//    
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        size_t readaddr = region->address + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {
//            for(size_t b = 0; b < readsize; b++) {
//                size_t matches = 0;
//                
//                // если данные совпадают или пропустить
//                while (buffer[b + matches] == pattern[matches] || mask[matches] != 'x') {
//                    matches++;
//                    
//                    if (matches == len) {
//                        found++;
//                        out->push_back((uintptr_t) (readaddr + b));
//                    }
//                }
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return found;
//}
//
//size_t
//Handle::scan_exact(vector<Entry> *out, 
//                   const region_t *region, 
//                   vector<Entry> entries, 
//                   size_t increment)
//{
//    byte buffer[0x1000];
//    
//    size_t chunksize = sizeof(buffer);
//    size_t totalsize = region->end - region->address;
//    size_t chunknum = 0;
//    size_t found = 0;
//    // TODO[HIGH] научить не добавлять, если предыдущий (собсна, наибольший) уже есть 
//    while (totalsize) {
//        size_t readsize = (totalsize < chunksize) ? totalsize : chunksize;
//        uintptr_t readaddr = region->address + (chunksize * chunknum);
//        bzero(buffer, chunksize);
//        
//        if (this->read(buffer, (void *) readaddr, readsize)) {    // read into buffer
//            for(uintptr_t b = 0; b < readsize; b += increment) {  // for each addr inside buffer
//                for(int k = 0; k < entries.size(); k++) {         // for each entry
//                    size_t matches = 0;
//                    while (buffer[b + matches] == entries[k].value.bytes[matches]) {  // находим адрес
//                        matches++;
//                        
//                        if (matches == SIZEOF_FLAG(entries[k].flags)) {
//                            found++;
//                            out->emplace_back(entries[k].flags, (uintptr_t)(readaddr + b), region, entries[k].value.bytes);
//                            //todo мне кажется, что нужно всё-таки добавить плавующую точку, посмотрим, как сделаю scan_reset
//                            goto sorry_for_goto;
//                        }
//                    }
//                }
//                sorry_for_goto: ;
//            }
//        }
//        
//        totalsize -= readsize;
//        chunknum++;
//    }
//    return found; //size of pushed back values
//}
