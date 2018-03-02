//
// Created by root on 12.02.18.
//


#include <iostream>
#include <Core/api.hh>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

const char *target = "HackMe";

int
main() {
    int N = 8;
    long SAMPLES = 100'000'000;
    high_resolution_clock::time_point t1, t2;
    duration<double> time_span;
    
    printf("N: %d\n", N);
    
    
    /// Method 6 done 10000000000 samples in 25.245812 seconds
    union byteint
    {
        byte b[sizeof(int)];
        int i;
    };
    
    t1 = high_resolution_clock::now();
    for(long ii=0; ii<SAMPLES; ii++) {
        byteint bi;
        bi.i = N;
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    
    byteint bi;
    bi.i = N;
    
    printf("byte6: %x %x %x %x, done %ld samples in %f seconds\n", bi.b[0], bi.b[1], bi.b[2], bi.b[3], SAMPLES, time_span.count());
    
    
    
    /// Method 2 done 10000000000 samples in 26.711245 seconds
    t1 = high_resolution_clock::now();
    for(long i=0; i<SAMPLES; i++) {
        unsigned char byte2[4];
        byte2[0] = (N & 0x000000ff);
        byte2[1] = (N & 0x0000ff00)>>8;
        byte2[2] = (N & 0x00ff0000)>>16;
        byte2[3] = (N & 0xff000000)>>24;
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    
    unsigned char byte2[4];
    byte2[0] = (N & 0x000000ff);
    byte2[1] = (N & 0x0000ff00)>>8;
    byte2[2] = (N & 0x00ff0000)>>16;
    byte2[3] = (N & 0xff000000)>>24;
    
    printf("byte2: %x %x %x %x, done %ld samples in %f seconds\n", byte2[0], byte2[1], byte2[2], byte2[3], SAMPLES, time_span.count());
    
    
    
    /// Method 1 done 10000000000 samples in 25.514802 seconds
    t1 = high_resolution_clock::now();
    for(long i=0; i<SAMPLES; i++) {
        unsigned char byte1[4];
        byte1[3] = (N>>24) & 0xFF;
        byte1[2] = (N>>16) & 0xFF;
        byte1[1] = (N>>8) & 0xFF;
        byte1[0] = N & 0xFF;
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    
    unsigned char byte1[4];
    byte1[3] = (N>>24) & 0xFF;
    byte1[2] = (N>>16) & 0xFF;
    byte1[1] = (N>>8) & 0xFF;
    byte1[0] = N & 0xFF;
    
    printf("byte1: %x %x %x %x, done %ld samples in %f seconds\n", byte1[0], byte1[1], byte1[2], byte1[3], SAMPLES, time_span.count());
    
    
    
    /// Method 4 done 10000000000 samples in 24.689415 seconds
    t1 = high_resolution_clock::now();
    for(long i=0; i<SAMPLES; i++) {
        unsigned char *byte4 = reinterpret_cast<unsigned char *>(&N);
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    
    unsigned char *byte4 = reinterpret_cast<unsigned char *>(&N);
    
    printf("byte4: %x %x %x %x, done %ld samples in %f seconds\n", byte4[0], byte4[1], byte4[2], byte4[3], SAMPLES, time_span.count());
    
    
    
    
    
    /*    Handle *h = nullptr;
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
        
        clog<<"output: "<<output<<endl;*/
    
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


























