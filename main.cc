#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <libreverseengine/value.hh>
#include <bitset>


int main()
{
    using namespace std;
    uint8_t c = '\xf0';
    int res = (c << 24) | (c << 16) | (c << 8) | c;
    clog<<HEX(res)<<endl;
    
//    char flags;
//    flags = readable | writable | executable | shared;
//    clog<<"flags: "<<bitset<4>(flags)<<"\n"
//        <<"flags: "<<bitset<4>(flags ^ (readable | writable | ~shared))<<", bool: "<<(bool)((flags ^ (readable | writable | ~shared))!=0)<<"\n"
//        <<endl;
//    
//    flags = readable | writable | executable;
//    clog<<"flags: "<<bitset<4>(flags)<<"\n"
//        <<"flags: "<<bitset<4>(flags ^ (readable | writable | ~shared))<<", bool: "<<(bool)((flags ^ (readable | writable | ~shared))!=0)<<"\n"
//        <<endl;
//    
//    flags = readable | writable;
//    clog<<"flags: "<<bitset<4>(flags)<<"\n"
//        <<"flags: "<<bitset<4>(flags ^ (readable | writable | ~shared))<<", bool: "<<(bool)((flags ^ (readable | writable | ~shared))!=0)<<"\n"
//        <<endl;
//    
}
