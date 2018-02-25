//
// Created by root on 12.02.18.
//


#include <iostream>
#include <Core/api.hh>
#include <iomanip>

using namespace std;

#define HEX(a) hex<<(a)<<dec

const char *target = "HackMe";

int
main() {
    clog<<"Begin"<<endl;

    Handle *h = nullptr;
    region_t *entry = nullptr;
    region_t *libc = nullptr;
    region_t *ld = nullptr;


    clog<<"Waiting for ["<<target<<"] process"<<endl;
    for(;;) {
        delete h;
        h = new Handle(target);
        if (h->isRunning())
            break;
        usleep(500'000);
    }
    clog<<"Found! PID is ["<<h->pid<<"]"<<endl; //как не переводить в хекс и обратно?

    for(;;) {
        h->updateRegions();
        entry = h->getRegion();
        libc = h->getRegion("libc-2.26.so");
        ld = h->getRegion("ld-2.26.so");
        if (entry && libc && ld)
            break;
        usleep(500'000);
    }
    clog<<"Found ["<<entry->filename<<"] at ["<<HEX(entry->start)<<"]"<<endl;
    clog<<"Found ["<<libc->filename<<"] at ["<<HEX(libc->start)<<"]"<<endl;
    clog<<"Found ["<<ld->filename<<"] at ["<<HEX(ld->start)<<"]"<<endl;

    
    size_t len = 32;
    void *v = new char[len];    
    if (h->findPointer(v, ld->start + 0x00225f50, {0x20, 0x40, 0x78, 0x238, 0x8}, len))
        clog<<"v is ["<<*(float *)(v)<<"]"<<endl;
    else
        clog<<"FAIL"<<endl;
    clog<<"v in hex: "<<HEX(*(int *)(v))<<endl;
    string s((char *)v,len);
    clog<<"unconverted s: "<<HEX(*(int *)(v))<<endl;
    
    static const char* const lut = "0123456789ABCDEF";
    string output;
    output.reserve(2 * len);
    
    for(int i=0; i<len; i++) {
        const unsigned char c = s[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    
    clog<<"output: "<<output<<endl;
    
//    bool upper_case = false;
//    ostringstream ret;
//
//    for (string::size_type i = 0; i < v.length(); ++i)
//        ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)v[i];
//
//    clog<<"ret: "<<ret.str()<<endl;
    
    
    
//    vector<uintptr_t> sig;
//    size_t found = h->findPattern(&sig, ld,
//                   "\x48\x89\xC7\xE8\x00\x00\x00\x00",
//                   "xxxx????");
//    clog<<"found: "<<found<<endl;
//    if (found > 0 && found < 20)
//        for(uintptr_t a : sig) { 
//            clog<<"\tat 0x"<<HEX(a)<<endl;
//        }


    return 0;
}


























