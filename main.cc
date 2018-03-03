//
// Created by root on 12.02.18.
//


//#include <variant> //не работает нихуя...

#include <iostream>
#include <Core/api.hh>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace std::literals;

const char *target = "HackMe";

int
main() {
//    float N = 8;
//    long SAMPLES = 100'000'000;
//    high_resolution_clock::time_point t1, t2;
//    duration<double> time_span;
//
//    printf("N: %f\n", N);
//
//    /// reinterpret_cast vs. union holywar:
//    // Method "byte *ARRAY = reinterpret_cast<byte *>(&NUMBER);" done 10000000000 samples in 24.689415 seconds
//    // Method "union u {byte b[sizeof(T)], T a};" done 10000000000 samples in 25.245812 seconds
//    
//    t1 = high_resolution_clock::now();
//    for(long ii=0; ii<SAMPLES; ii++) {
//        boost::variant<byte *, float> u = N;
//    }
//    t2 = high_resolution_clock::now();
//    time_span = duration_cast<duration<double>>(t2 - t1);
    float n = 8;
    byte *a = reinterpret_cast<byte *>(&n);
    printf("a: %x %x %x %x\n",
           a[3], 
           a[2], 
           a[1], 
           a[0]);
    
    
//    boost::variant<float, byte *> u; boost::variant std::variant хуйня ебаная, union лучше
//    u = n;
//    cout<<"float: "<<u<<endl;
//    byte *b = boost::get<byte *>(u);
//    printf("b: %x %x %x %x\n",
//           b[3], 
//           b[2], 
//           b[1], 
//           b[0]);
    
//    variant<byte *, float> u = N;
//    
//    
//    printf("byte6: %x %x %x %x, done %ld samples in %f seconds\n",
//           get<byte *>(u)[3], 
//           get<byte *>(u)[2], 
//           get<byte *>(u)[1], 
//           get<byte *>(u)[0], 
//           SAMPLES, time_span.count());
    
    
    
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


























