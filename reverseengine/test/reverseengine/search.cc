//
// Created by root on 28.07.18.
//

#include <iostream>
#include <string>
#include <chrono>
#include <reverseengine/core.hh>
#include <reverseengine/scanner.hh>


int main(int argc, const char *argv[]) {
    using std::cout, std::clog, std::cerr, std::endl;
    using namespace std::chrono;
    high_resolution_clock::time_point timestamp;
    { std::fstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary); if (f.is_open()) f<<"700"; }

    //sleep(100000);
    //return 0;

    std::string target = "FAKEMEM";
    // std::string target = "csgo_linux64";
    std::string search_for = "100";
    // RE::Edata_type data_type = RE::Edata_type::ANYNUMBER;
    RE::Edata_type data_type = RE::Edata_type::FLOAT64;

    RE::Process handler(target);
    if (!handler)
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");

    RE::Scanner scanner(&handler);

    clog<<"FAKEMEM, pid: "
        <<handler.get_pid()
        <<", title: "<< handler.get_cmdline()
        <<", exe: "<<handler.get_exe()
        <<", executable: "<<handler.get_executable()
        <<endl;


    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        scanner.string_to_uservalue(data_type, search_for, &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        return 0;
    }

    RE::matches_t matches_first;
    timestamp = high_resolution_clock::now();
    scanner.scan_regions(matches_first, data_type, uservalue, match_type);
    clog<<"Scan 1/3 done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds"<<endl;
    clog<<"mem_virt: "<<matches_first.mem_virt()<<endl;
    clog<<"mem_disk: "<<matches_first.mem_disk()<<endl;
    clog<<"size: "<<matches_first.size()<<endl;
    clog<<"count: "<<matches_first.count()<<endl;

    RE::matches_t matches_prev = matches_first;
    timestamp = high_resolution_clock::now();
    scanner.scan_update(matches_prev);
    scanner.scan_recheck(matches_prev, data_type, uservalue, match_type);
    clog<<"Scan 2/3 done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds"<<endl;
    clog<<"mem_virt: "<<matches_prev.mem_virt()<<endl;
    clog<<"mem_allo: "<<matches_prev.mem_allo()<<endl;
    clog<<"mem_disk: "<<matches_prev.mem_disk()<<endl;
    clog<<"size: "<<matches_prev.size()<<endl;
    clog<<"count: "<<matches_prev.count()<<endl;


    clog<<"============================================="<<endl;


    timestamp = high_resolution_clock::now();
    // RE::phandler_file handler_mmap(handler, "vadimislove");
    RE::ProcessH handler_mmap(handler);
    if (!handler_mmap)
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");
    handler_mmap.update_regions();
    RE::Scanner scanner_mmap(&handler_mmap);


    RE::matches_t matches_curr = matches_prev;
    scanner_mmap.scan_update(matches_curr);
    scanner_mmap.scan_recheck(matches_curr, data_type, uservalue, match_type);
    clog<<"Scan 3/3 done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds"<<endl;
    clog<<"mem_virt: "<<matches_curr.mem_virt()<<endl;
    clog<<"mem_allo: "<<matches_curr.mem_allo()<<endl;
    clog<<"mem_disk: "<<matches_curr.mem_disk()<<endl;
    clog<<"size: "<<matches_curr.size()<<endl;
    clog<<"count: "<<matches_curr.count()<<endl;

    return 0;
}
