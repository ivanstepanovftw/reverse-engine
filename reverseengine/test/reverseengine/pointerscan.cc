//
// Created by root on 23.03.19.
//

#include <iostream>
#include <string>
#include <chrono>
#include <reverseengine/core.hh>
#include <reverseengine/scanner.hh>
#include <reverseengine/pointerscan.hh>
#include <reverseengine/value.hh>


int main(int argc, const char *argv[]) {
    using std::cout, std::clog, std::cerr, std::endl;
    using namespace std::chrono;
    high_resolution_clock::time_point timestamp;

    {
        std::fstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary);
        if (f.is_open())
            f<<"700";
    }

    // std::string target = "csgo_linux64";
    // std::string target = "PwnAdventure3-Linux-Shipping";
    std::string target = "FAKEMEM";

    RE::Process handler(target);
    if (!handler)
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");

    clog<<target<<", pid: "
            <<handler.get_pid()
            <<", title: "<< handler.get_cmdline()
            <<", executable: "<<handler.get_executable()
            <<endl;
    clog.flush();

    // ----------------------------------------------------------
    // std::string pointer_region = "libtier0_client.so";
    // std::vector<uintptr_t> pointer_offset {
    //     0x6cc0, // 5f32 2e33 2e34 // "_2.3.4"
    // };
    // cout<<"pos_z: "<<*reinterpret_cast<float*>(&value)<<endl;
    // ----------------------------------------------------------
    // std::string pointer_region = "PwnAdventure3-Linux-Shipping";
    // std::vector<ptrdiff_t> pointer_offset {
    //         0x034158D8,
    //         0x5F0,
    //         0x8,
    //         0x250,
    //         0x130,
    //         0xC8,
    // };
    // cout<<"pos_z: "<<value<<endl;
    // ----------------------------------------------------------
    //std::string pointer_region = "FAKEMEM";
    // ----------------------------------------------------------

    // RE::Pointer p(handler, pointer_region, pointer_offset);

    // std::vector<uintptr_t> ptr_resolved = p.resolve();
    // if (pointer_offset.size() != ptr_resolved.size()) {
    //     cout<<"Pointer is not fully resolved: "<<pointer_offset.size() <<" != "<< ptr_resolved.size()<<endl;
    // } else {
    //     uint64_t value = 0;
    //     if (!handler.read(ptr_resolved.back(), &value)) {
    //         exit(1);
    //     }
    //     cout<<"pos_z: "<<reinterpret_cast<char*>(&value)<<endl;
    // }
    //
    // cout<<"ptr_resolved.back(): "<<RE::HEX(ptr_resolved.back())<<endl;

    // RE::PointerScanner ps(&handler);
    //or
    // RE::ProcessF processF(handler, "asd");
    // RE::PointerScanner ps(&processF);
    //or
    RE::ProcessH processH(handler);
    RE::PointerScanner ps(processH);

    uintptr_t seek = handler.get_region_by_name(target)->address+0x10;
    cout<<"seek: "<<RE::HEX(seek)<<endl;
    // std::vector<RE::RegionPaths> scan_result = ps.scan_regions(0x7f50dc812000);
    std::vector<RE::RegionPaths> scan_result = ps.scan_regions(seek); //fakemem`begin+0x10 //43 pointera
    // std::vector<RE::RegionPaths> scan_result = ps.scan_regions(0x7FFCA82F56C0); //fakemem's bytes allocated

    // *(*("FAKEMEM"+302100)+4)+40
    // "FAKEMEM"+302100->+4->+40
    cout<<"pointer list:"<<endl;
    for (const RE::RegionPaths& region : scan_result) {
        for(const std::vector<uintptr_t>& offsets : region.paths) {
            cout<<region.file.filename()<<"+";
            for(const uintptr_t& o : offsets) {
                cout<<RE::HEX(o)<<"+";
            }
            cout<<"\b"<<endl;
        }
    }

    cout<<"No error"<<endl;
    return 0;
}
