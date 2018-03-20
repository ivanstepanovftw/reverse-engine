//
// Created by root on 12.02.18.
//

#include <iostream>
#include <iomanip>
#include <chrono>
#include <type_traits>

using namespace std;
using namespace std::chrono;
using namespace std::literals;

const char *target = "HackMe";

#define HEX(s) hex<<(s)<<dec


#define LIFT_ENUM_OP(OP, ASSIGNOP)                                              \
    template <typename E, typename U = typename std::underlying_type<E>::type>  \
    static inline E operator OP (E lhs, E rhs)                                  \
    { return static_cast<E>(static_cast<U>(lhs) OP static_cast<U>(rhs)); }      \
                                                                                \
    template <typename E, typename U = typename std::underlying_type<E>::type>  \
    static inline E& operator ASSIGNOP (E& lhs, E rhs)                          \
    { lhs = lhs OP rhs; return lhs; }

template <typename E, typename U = typename std::underlying_type<E>::type>
static inline E operator ~ (E lhs)
{ return static_cast<E>(~static_cast<U>(lhs)); }
LIFT_ENUM_OP(&, &=)
LIFT_ENUM_OP(|, |=)
LIFT_ENUM_OP(^, ^=)

#undef LIFT_ENUM_OP


enum class Flags : uint16_t
{
    empty = 0,
    u1 = 1 << 0,  // could be an unsigned  8-bit variable (e.g. unsigned char)
    s1 = 1 << 1,  // could be a    signed  8-bit variable (e.g. signed char)
    u2 = 1 << 2,  // could be an unsigned 16-bit variable (e.g. unsigned short)
    s2 = 1 << 3,  // could be a    signed 16-bit variable (e.g. short)
    u4 = 1 << 4,  // could be an unsigned 32-bit variable (e.g. unsigned int)
    s4 = 1 << 5,  // could be a    signed 32-bit variable (e.g. int)
    u8 = 1 << 6,  // could be an unsigned 64-bit variable (e.g. unsigned long long)
    s8 = 1 << 7,  // could be a    signed 64-bit variable (e.g. long long)
    f4 = 1 << 8,  // could be a 32-bit floating point variable (i.e. float)
    f8 = 1 << 9,  // could be a 64-bit floating point variable (i.e. double)
    full = 0xffff
};

static inline size_t flags_size(Flags flags)
{
    switch (flags) {
        case Flags::u8:
        case Flags::s8:
        case Flags::f8: return 8;
        case Flags::u4:
        case Flags::s4:
        case Flags::f4: return 4;
        case Flags::u2:
        case Flags::s2: return 2;
        case Flags::u1:
        case Flags::s1: return 1;
        default:        return 0;
    }
}

union memory {
    uint8_t     u1;
    int8_t      s1;
    uint16_t    u2;
    int16_t     s2;
    uint32_t    u4;
    int32_t     s4;
    uint64_t    u8;
    int64_t     s8;
    float       f4;
    double      f8;
    uint8_t     bytes[sizeof(int64_t)];
};

class Pattern {
public:
    Flags flags = Flags::empty;
    
    memory u1;
    memory s1;
    memory u2;
    memory s2;
    memory u4;
    memory s4;
    memory u8;
    memory s8;
    memory f4;
    memory f8;
    
    memory* operator[](size_t idx) {
        switch(idx) {
            case 0: return &u1;
            case 1: return &s1;
            case 2: return &u2;
            case 3: return &s2;
            case 4: return &u4;
            case 5: return &s4;
            case 6: return &u8;
            case 7: return &s8;
            case 8: return &f4;
            case 9: return &f8;
            default:return nullptr;
        }
    }
};

int
main() {
    /// Parse user value
    Pattern p;
    p.flags = Flags::s8 | Flags::u4;
    p.s8.s8 = 0;
    p.u4.u4 = 0;
    
    /// Get memory chunk
    memory *m = new memory;
    for(int i=0; i<8; i++)
        m->bytes[i] = 0;
    
    /// Scan routine
    Flags flags = Flags::empty;
    for(int i=0; i<=9; i++) {
        if ((p.flags & static_cast<Flags>(1 << i)) && m->bytes == p[i]->bytes) flags |= static_cast<Flags>(1 << i);
    }
    
    
//    float N = 8;
//    long SAMPLES = 100'000'000;
//    high_resolution_clock::time_point t1, t2;
//    duration<double> time_span;
//
//    printf("N: %f\data", N);
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

//    double a = -1./0.;
//    double b = 0./0. ;
//    double c = 1./0. ;
//    clog<<"-1./0.  = "<<a<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&a)<<dec<<endl;
//    clog<<"0./0.   = "<<b<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&b)<<dec<<endl;
//    clog<<"1./0.   = "<<c<<" = "<<hex<<*reinterpret_cast<uint64_t *>(&c)<<dec<<endl;

//    Handle *h = nullptr;
//    region_t *entry = nullptr;
//    region_t *libc = nullptr;
//    region_t *ld = nullptr;
//
//
//    clog<<"Waiting for ["<<target<<"] process"<<endl;
//    for(;;) {
//        delete h;
//        h = new Handle(target);
//        if (h->isRunning())
//            break;
//        usleep(500'000);
//    }
//    clog<<"Found! PID is ["<<h->pid<<"]"<<endl; //как не переводить в хекс и обратно?
//
//    for(;;) {
//        h->updateRegions();
//        entry = h->getRegion();
//        libc = h->getRegion("libc-2.26.so");
//        ld = h->getRegion("ld-2.26.so");
//        if (entry && libc && ld)
//            break;
//        usleep(500'000);
//    }
//    clog<<"Found ["<<entry->filename<<"] at ["<<HEX(entry->start)<<"]"<<endl;
//    clog<<"Found ["<<libc->filename<<"] at ["<<HEX(libc->start)<<"]"<<endl;
//    clog<<"Found ["<<ld->filename<<"] at ["<<HEX(ld->start)<<"]"<<endl;
//    
//    region_t *aas = h->getRegionOfAddress(ld->start+8);
//    clog<<"aas: "<<aas->filename.c_str()<<endl;
    
//    
//    
//    
//    
//    
//    size_t len = 32;
//    void *v = new char[len];
//    clog<<"v in hex: "<<HEX(*(int *)(v))<<endl;
//    string s((char *)v,len);
//    clog<<"unconverted s: "<<HEX(*(int *)(v))<<endl;
//    
//    static const char* const lut = "0123456789ABCDEF";
//    string output;
//    output.reserve(2 * len);
//    
//    for(int i=0; i<len; i++) {
//        const unsigned char c = s[i];
//        output.push_back(lut[c >> 4]);
//        output.push_back(lut[c & 15]);
//    }
//    
//    clog<<"output: "<<output<<endl;
    
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


























