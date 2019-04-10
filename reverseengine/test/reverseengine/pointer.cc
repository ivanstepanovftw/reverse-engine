//
// Created by root on 23.03.19.
//

#include <iostream>
#include <string>
#include <chrono>
#include <reverseengine/core.hh>
#include <reverseengine/scanner.hh>
#include <reverseengine/pointer.hh>
#include <reverseengine/value.hh>


int main() {
    using std::cout, std::clog, std::cerr, std::endl;
    using namespace std::chrono;
    high_resolution_clock::time_point timestamp;

    {
        std::fstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary);
        if (f.is_open())
            f<<"700";
    }

    std::string target = "csgo_linux64";
    // std::string target = "PwnAdventure3-Linux-Shipping";
    //std::string target = "FAKEMEM";

    RE::phandler_pid handler(target);
    if (!handler)
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");

    clog<<target<<", pid: "
            <<handler.get_pid()
            <<", title: "<< handler.get_cmdline()
            <<", exe: "<<handler.get_exe()
            <<", executable: "<<handler.get_executable()
            <<endl;
    clog.flush();

    // ----------------------------------------------------------
    std::string pointer_region = "libtier0_client.so";
    std::vector<ptrdiff_t> pointer_offset {
        0x6cbf, // 5f32 2e33 2e34 // "_2.3.4"
    };
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

    // RE::pointer p(handler, pointer_region, pointer_offset);

    // std::vector<uintptr_t> ptr_resolved = p.resolve();
    // cout<<p.str(ptr_resolved)<<endl;


    uintptr_t pointer = handler.get_region_by_name(pointer_region)->address+0x6cc0;

    uint64_t value = 0;
    // if (!handler.read(ptr_resolved.back(), &value)) {
    //     exit(1);
    // }
    if (!handler.read(pointer, &value)) {
        exit(1);
    }
    cout<<"pos_z: "<<reinterpret_cast<char*>(&value)<<endl;

    // RE::region *main_region = handler.get_region_by_name("PwnAdventure3-Linux-Shipping");
    // cout<<"region_orig: "<<*main_region<<endl;
    // RE::region *main_plus = handler.get_region_of_address(main_region->address+0x034158D8);
    // cout<<"region_orig+0x034158D8: "<<*main_plus<<endl;

    //cout<<"ptr_resolved.back(): "<<RE::HEX(ptr_resolved.back())<<endl;
    // RE::phandler_file hndlr(handler, "asd");
    // RE::pointerscan ps(&handler);
    // std::vector<RE::ptr> a = ps.scan_regions(0, 2048, 5);
    //std::vector<RE::ptr> a = ps.scan_regions(ptr_resolved.back(), 2048, 5);

    return 0;
}
