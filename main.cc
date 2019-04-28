// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com.
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <memory>
#include <regex>
#include <cmath>
#include <iterator>
#include <zconf.h>
#include <filesystem>
#include <reverseengine/r2_wrapper.hh>
#include <reverseengine/scanner.hh>
#include <reverseengine/core.hh>


namespace sfs = std::filesystem;
// namespace bio = boost::iostreams;


class Timer {
public:
    explicit Timer(std::string what = "Timer")
            : m_what(std::move(what)), m_tp(std::chrono::high_resolution_clock::now()) {}

    ~Timer() {
        std::clog << m_what << ": done in " << std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - m_tp).count() << " seconds" << std::endl;
    }

private:
    std::string m_what;
    std::chrono::high_resolution_clock::time_point m_tp;
};

//
// #define __device__
// #define __global__
//
// /// ------------
// /// bigalloc.cuh
// /// ------------
// extern __device__ void *balloc(int size);
//
// /// -----------
// /// bigalloc.cu
// /// -----------
// //This file is used when a lot of memory needs to be allocated in small chunks
// //Note: this is NOT thread safe, only let 1 thread access this code
// #pragma pack 16
// typedef struct _BigAllocs
// // typedef __declspec(align(16)) struct _BigAllocs
// {
//     int totalsize;
//     int pos;
//     unsigned char *buffer;
// } BigAllocs, *PBigAllocs;
//
// __device__ BigAllocs *allocs = NULL;
// __device__ int allocsPos = 0;
// __device__ int allocsMax = 0;
//
// __device__ void *balloc(int size) {
//     void *result = NULL;
//
//     //printf("allocating %d bytes\n", size);
//
//     if (allocs == NULL) {
//         //allocate a big amoung of memory first
//         printf("balloc first init\n");
//         allocsMax = 16;
//         allocsPos = 0;
//         allocs = (PBigAllocs) malloc(allocsMax * sizeof(BigAllocs));
//         allocs[allocsPos].pos = 0;
//         allocs[allocsPos].totalsize = 2 * 1024 * 1024;
//         allocs[allocsPos].buffer = (unsigned char *) malloc(allocs[allocsPos].totalsize); //16MB	+1mb for each new pos
//         printf("allocs[allocsPos].buffer=%p\n", allocs[allocsPos].buffer);
//         memset(allocs[allocsPos].buffer, 0, allocs[allocsPos].totalsize);
//     }
//
//     if (allocs[allocsPos].totalsize - allocs[allocsPos].pos < size) {
//         printf("balloc reinit\n");
//
//         //a new BigAllocs object is needed
//         allocsPos++;
//         if (allocsPos >= allocsMax) {
//             PBigAllocs old = allocs;
//             printf("Reallocating\n");
//
//             allocs = (PBigAllocs) malloc(allocsMax * sizeof(BigAllocs) * 2);
//             memcpy(allocs, old, allocsMax * sizeof(BigAllocs));
//
//             allocsMax *= 2; //allocate more blocks
//         }
//
//         allocs[allocsPos].pos = 0;
//         allocs[allocsPos].totalsize = 2 * 1024 * 1024;
//         allocs[allocsPos].buffer = (unsigned char *) malloc(allocs[allocsPos].totalsize);
//         memset(allocs[allocsPos].buffer, 0, allocs[allocsPos].totalsize);
//
//         printf("buffer=%p\n", allocs[allocsPos].buffer);
//     }
//
//     result = &allocs[allocsPos].buffer[allocs[allocsPos].pos];
//     allocs[allocsPos].pos += size;
//     if (allocs[allocsPos].pos & 0xf) //make sure the next one is aligned
//         allocs[allocsPos].pos = (allocs[allocsPos].pos + 0x10) & ~(0xf);
//
//
//     //printf("allocs[allocsPos].pos=%x\n", allocs[allocsPos].pos);
//
//     return result;
// }
//
//
// /// -----------
// /// cudapointervaluelist.cuh
// /// -----------
// typedef struct {
//     uint32_t moduleindex;
//     int offset;
// } TStaticData, *PStaticData;
//
// typedef struct {
//     uintptr_t address;
//     PStaticData staticdata;
// } TPointerData, *PPointerData;
//
// typedef struct _PointerList {
//     // int maxsize;  //not needed for preloaded scandata (saves some space)
//     //int expectedsize;
//     int pos;
//     PPointerData list;
//
//     //Linked list
//     uintptr_t PointerValue;
//     _PointerList *Previous;
//     _PointerList *Next;
//
// } TPointerList, *PPointerList;
//
// typedef struct _ReversePointerList {
//     union {
//         PPointerList PointerList;
//         _ReversePointerList *ReversePointerList;
//     } u;
// } TReversePointerList, *PReversePointerList;
//
//
// extern __device__ PPointerList findPointerValue(uintptr_t startvalue, uintptr_t *stopvalue);
//
// __global__ void findoraddpointervalue(unsigned char *bla, int max);
//
// __global__ void generateLinkedList(void);
//
// void setMaxLevel(int count);
//
//
// /// -----------------------
// /// cudapointervaluelist.cu
// /// -----------------------
//
// __device__ PReversePointerList Level0List = NULL;
// __device__ int maxlevel;
//
// __device__ PPointerList firstPointerValue = NULL;
// __device__ PPointerList lastPointerValue = NULL;
//
// __device__ PPointerList findClosestPointer(PReversePointerList addresslist, int entrynr, int level, uintptr_t maxvalue) {
//     /// The pointer was not found exactly, but we are in an addresslist that has been allocated, so something is filled in at least
//     int i;
//     PPointerList result = NULL;
//
//     for (i = entrynr + 1; i <= 0xF; i++) {
//         if (addresslist[i].u.ReversePointerList) {
//             if (level == maxlevel) {
//                 result = addresslist[i].u.PointerList;
//                 while ((result) && (result->PointerValue > maxvalue)) //should only run one time
//                     result = result->Previous;
//
//                 if (result == NULL)
//                     result = firstPointerValue;
//
//                 return result;
//             } else {
//                 //dig deeper
//                 result = findClosestPointer(addresslist[i].u.ReversePointerList, -1, level + 1,
//                                             maxvalue); //so it will be found by the next top scan
//                 if (result)
//                     return result;
//             }
//         }
//     }
//
//     //nothing at the top, try the bottom
//     for (i = entrynr - 1; i >= 0; i--) {
//         if (addresslist[i].u.ReversePointerList) {
//             if (level == maxlevel) {
//                 result = addresslist[i].u.PointerList;
//                 while ((result) && (result->PointerValue > maxvalue)) //should never happen
//                     result = result->Previous;
//
//                 if (result == NULL)
//                     result = firstPointerValue;
//
//                 return result;
//
//             } else //dig deeper
//             {
//                 result = findClosestPointer(addresslist[i].u.ReversePointerList, 0x10, level + 1,
//                                             maxvalue); //F downto 0
//                 if (result)
//                     return result;
//             }
//         }
//     }
//     return result;
//
//
// }
//
// __device__ PPointerList findPointerValue(uintptr_t startvalue, uintptr_t *stopvalue)
// {
//     /// find a node that falls in the region of stopvalue and startvalue
//     PPointerList result = NULL;
//     int level;
//     PReversePointerList currentarray;
//     int entrynr;
//     uintptr_t _stopvalue;
//
//     _stopvalue = *stopvalue;
//     currentarray = Level0List;
//
//     // printf("findPointerValue for %x\n", (unsigned int)startvalue);
//     // printf("maxlevel is %d\n", (unsigned int)maxlevel);
//
//     for (level = 0; level <= maxlevel; level++) {
//         entrynr = ((uint64_t) _stopvalue >> (uint64_t)(((maxlevel - level) * 4))) & 0xf;
//
//
//         if (currentarray[entrynr].u.ReversePointerList == NULL) {
//             //not found
//             result = findClosestPointer(currentarray, entrynr, level, _stopvalue);
//             break;
//         } else {
//             if (level == maxlevel) {
//                 result = currentarray[entrynr].u.PointerList;
//                 break;
//             }
//         }
//         currentarray = currentarray[entrynr].u.ReversePointerList;
//     }
//     *stopvalue = result->PointerValue;
//     //clean up bad results
//     // printf("result=%p\n", result);
//     // printf("result->PointerValue=%x\n", (unsigned int)result->PointerValue);\
//     // printf("result->Next->PointerValue=%x\n", (unsigned int)result->Next->PointerValue);
//
//     if (result->PointerValue < startvalue)
//         result = NULL;
//
//     return result;
// }
//
//
// __device__ void fillList(PReversePointerList addresslist, int level, PPointerList *prev)
// /*
// Fills in the linked list of the reverse pointer list
// */
// {
//     int i;
//     if (level == maxlevel) {
//         for (i = 0; i <= 0xf; i++) {
//             if (addresslist[i].u.PointerList) {
//                 if (*prev)
//                     (*prev)->Next = addresslist[i].u.PointerList;
//                 else
//                     firstPointerValue = addresslist[i].u.PointerList;
//
//                 addresslist[i].u.PointerList->Previous = *prev;
//                 *prev = addresslist[i].u.PointerList;
//             }
//         }
//     } else {
//         for (i = 0; i <= 0xf; i++) {
//             if (addresslist[i].u.ReversePointerList)
//                 fillList(addresslist[i].u.ReversePointerList, level + 1, prev);
//         }
//     }
//
// }
//
// __global__ void generateLinkedList(void) {
//     lastPointerValue = NULL;
//     fillList(Level0List, 0, &lastPointerValue);
// }
//
// __global__ void findoraddpointervalue(unsigned char *bla, int max) {
//     // Go through the data and add the pointervalue. (port from ce's pascal source, with some improvements since no dynamic loading is necesary)
//     int i;
//     int loopnr;
//     int pd = 0;
//     uint64_t pointervalue;
//     uint32_t pointercount;
//     int level, entrynr, size;
//     PReversePointerList currentarray, temp;
//     PPointerList plist;
//
//     if (Level0List == NULL) {
//         //first time init
//         Level0List = (PReversePointerList) malloc(16 * sizeof(PReversePointerList));
//         memset(Level0List, 0, 16 * sizeof(PReversePointerList));
//     }
//
//     // printf("this one will crash\n");
//     loopnr = 1;
//
//     while (pd < max) {
//         // printf("loopnr %d\n", loopnr);
//         memcpy(&pointervalue, &bla[pd], sizeof(pointervalue));
//         pd += sizeof(pointervalue);
//         // printf("pointervalue=%x\n", (unsigned int)pointervalue);
//         currentarray = Level0List;
//         level = 0;
//
//         while (level < maxlevel) {
//             //add the path if needed
//             entrynr = ((uint64_t) pointervalue >> (uint64_t)(((maxlevel - level) * 4))) & 0xf;
//
//             if (currentarray[entrynr].u.ReversePointerList == NULL) {
//                 //allocate
//                 size = 16 * sizeof(PReversePointerList);
//                 temp = (PReversePointerList) balloc(size);
//                 // memset(temp, 0, size);
//                 currentarray[entrynr].u.ReversePointerList = temp;
//             }
//             currentarray = currentarray[entrynr].u.ReversePointerList;
//             level++;
//         }
//
//         entrynr = ((uint64_t) pointervalue >> (uint64_t)(((maxlevel - level) * 4))) & 0xf;
//         plist = currentarray[entrynr].u.PointerList;
//
//         if (plist == NULL) {
//             //allocate one
//             currentarray[entrynr].u.PointerList = (TPointerList *) balloc(sizeof(TPointerList));
//             plist = currentarray[entrynr].u.PointerList;
//             plist->PointerValue = pointervalue;
//             plist->list = NULL;
//             plist->pos = 0;
//         }
//         //use the current plist
//         memcpy(&pointercount, &bla[pd], sizeof(pointercount));
//         pd += sizeof(pointercount);
//         plist->pos = pointercount;
//         plist->list = (TPointerData *) balloc(sizeof(TPointerData) * pointercount);
//         //printf("plist->list=%p\n", plist->list);
//         for (i = 0; i < pointercount; i++) {
//             uint64_t address;
//             memcpy(&address, &bla[pd], sizeof(address));
//             pd += sizeof(address);
//             plist->list[i].address = address;
//             if (bla[pd] == 1) {
//                 uint32_t moduleindex;
//                 uint32_t offset;
//                 pd += 1;
//                 memcpy(&moduleindex, &bla[pd], sizeof(moduleindex));
//                 pd += sizeof(moduleindex);
//                 memcpy(&offset, &bla[pd], sizeof(offset));
//                 pd += sizeof(offset);
//                 plist->list[i].staticdata = (TStaticData *) balloc(sizeof(TStaticData));
//                 plist->list[i].staticdata->moduleindex = moduleindex;
//                 plist->list[i].staticdata->offset = offset;
//                 //printf("plist->list[i].staticdata=%p\n", plist->list[i].staticdata);
//             } else {
//                 pd += 1;
//                 plist->list[i].staticdata = NULL;
//             }
//         } //for
//         // printf("pd=%d  max=%d\n", pd, max);
//         loopnr++;
//     }
// }
//
// void setMaxLevel(int count) {
//     // cudaMemcpyToSymbol(maxlevel, &count, sizeof(count));
//     maxlevel = count;
// }
//
// /// -------------------
// /// PointerScanner.cuh
// /// -------------------
// int PointerScanner(uintptr_t address, int structsize, int maxlevel);
//
//
// /// -------------------
// /// PointerScanner.cu
// /// -------------------
//
// #define MAXCOMMANDLISTSIZE	2048
// #pragma pack(16)
//
// // typedef __declspec(align(16)) struct _rcaller //recursion replacement
// typedef struct _rcaller //recursion replacement
// {
//     uintptr_t valueToFind;
//     uintptr_t startvalue;
//     uintptr_t stopvalue;
//     PPointerList plist;
//     int plistIndex;  //index in the plist to start off with
// } rcaller, *prcaller;
//
// // typedef __declspec(align(16)) struct _workcommand  //same as continuedata but no plist data
// typedef struct _workcommand  //same as continuedata but no plist data
// {
//     uintptr_t valueToFind;
//     int level;
//     int *offsets;
// } WorkCommand, *PWorkCommand;
//
//
// __global__ void pscan(PWorkCommand queueElements, PWorkCommand staticoutputqueue, int staticoutputquesize, PWorkCommand *allocatedoutputqueue, int *allocatedoutputsize)
// {
//     ///The pointerscanner iteration
//     // int index = threadIdx.x;// 0; // blockIdx.x * blockDim.x + threadIdx.x;
//     int index = 0;
//     uintptr_t stopValue = queueElements[index].valueToFind + 4096;
//     PPointerList pl;
//     int i;
//
//     pl = findPointerValue(queueElements[index].valueToFind, &stopValue);
//
//     if (pl == NULL)
//     {
//         staticoutputqueue[index].level = 666;
//         staticoutputqueue[index].offsets = 0;
//     }
//     else
//     {
//         staticoutputqueue[index].level = 777;
//         staticoutputqueue[index].offsets = 0;
//     }
// }
//
//
//
// int PointerScanner(uintptr_t address, int structsize, int maxlevel) {
//     PWorkCommand wc = (PWorkCommand)malloc(sizeof(WorkCommand) * 1024);
//     int wcsize = 1;
//     int i=0;
//     bool done=false;
//     // cudaError_t err;
//
//     //loop till all are done
//     while (!done){
//         if (i%10==0){
//             int r=0;
//             err=cudaMemcpyToSymbol(didWork, &r, sizeof(r));
//             if (err!=cudaSuccess)  {
//                 printf("CUDA error: %s\n", cudaGetErrorString(err));
//                 break;
//             }
//
//             r=12;
//             err=cudaMemcpyFromSymbol(&r, didWork, sizeof(r));
//             if (err!=cudaSuccess)  {
//                 printf("CUDA error: %s\n", cudaGetErrorString(err));
//                 break;
//             }
//
//             if (r!=0) {
//                 printf("FAIL\n");
//                 break;
//             }
//         }
//         //  printf("------------SCAN %d------------------\n", i);
//         pscan<<<1,1024>>>(cd, structsize, 5);
//         cudaDeviceSynchronize();
//
//         err=cudaGetLastError();
//
//         if (err!=cudaSuccess)   {
//             printf("CUDA error: %s\n", cudaGetErrorString(err));
//             break;
//         }
//
//         if (i%10==0) {
//             int r=0;
//             err=cudaMemcpyFromSymbol(&r, didWork, sizeof(r));
//             if (err!=cudaSuccess) {
//                 printf("CUDA error: %s\n", cudaGetErrorString(err));
//                 break;
//             }
//
//             if (r==0)
//                 done=TRUE;
//         }
//
//         i++;
//     } //loop
//
//     return 0;
// }
//
//




int main(int argc, const char *argv[], char *envp[]) {
    {
        std::fstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary);
        if (f.is_open()) f << "997";
    }
    using namespace std;
    namespace sfs = std::filesystem;

    Timer t("main");
    {
        std::vector<int> a;
        for(int b : a) {
            cout<<"B: "<<b<<endl;
        }
    }
    {
        RE::ByteMatches m0;
        for(RE::value_t a0 : m0) {
            throw runtime_error("");
        }
    }
    {
        RE::ByteMatches m0;
        m0.swaths.emplace_back(10);
        m0.swaths.back().append(10, RE::flag_t::flags_empty);
        // m0.swaths.back().append(20, RE::flag_t::flag_f32);
        m0.swaths.back().append(30, RE::flag_t::flags_empty);
        for(RE::value_t a0 : m0) {
            cout<<"bbbbbb!!!"<<endl;
        }
    }

    // {
    //     ///cudapscan.cu
    //     cudaDeviceSynchronize();
    //
    //     pscaninit();
    //     PointerScanner(0x00201C20, 2048, 5);
    //
    //     cudaDeviceSynchronize();
    //     cudaDeviceReset();
    // }

}
