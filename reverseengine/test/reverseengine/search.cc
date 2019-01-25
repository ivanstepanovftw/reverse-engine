//
// Created by root on 28.07.18.
//

#include <iostream>
#include <string>
#include <chrono>
#include <reverseengine/core.hh>
#include <reverseengine/scanner.hh>


int main() {
    using std::cout, std::clog, std::cerr, std::endl;
    using namespace std::chrono;
    high_resolution_clock::time_point timestamp;

    std::string target = "FAKEMEM";
    std::string search_for = "145";
    RE::Edata_type data_type = RE::Edata_type::ANYNUMBER;

    RE::Handle *handle = new RE::Handle(target);
    if (!handle->is_good())
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");
    handle->update_regions();
    RE::Scanner *scanner = new RE::Scanner(handle);


    clog<<"FAKEMEM, pid: "
        <<handle->pid
        <<", title: "<<handle->title
        <<endl;


    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        scanner->string_to_uservalue(data_type, search_for, &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        return 0;
    }

    RE::matches_t matches;

    timestamp = high_resolution_clock::now();
    scanner->scan_regions(matches, data_type, uservalue, match_type);
    clog<<"Scan 1/2 done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds"<<endl;
    clog<<"mem_virt: "<<matches.mem_virt()<<endl;
    clog<<"mem_disk: "<<matches.mem_disk()<<endl;
    clog<<"size: "<<matches.size()<<endl;
    clog<<"count: "<<matches.count()<<endl;

    timestamp = high_resolution_clock::now();
    scanner->scan_update(matches);
    scanner->scan_recheck(matches, data_type, uservalue, match_type);
    clog<<"Scan 2/2 done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()<<" seconds"<<endl;
    clog<<"mem_virt: "<<matches.mem_virt()<<endl;
    clog<<"mem_allo: "<<matches.mem_allo()<<endl;
    clog<<"mem_disk: "<<matches.mem_disk()<<endl;
    clog<<"size: "<<matches.size()<<endl;
    clog<<"count: "<<matches.count()<<endl;

    clog<<"============================================="<<endl;


    /*
     *  Сканируем из памяти в свою память           (scanmem)
     *      -> Сканируем из памяти в свою память    (scanmem)
     *  Сканируем из памяти в диск                  (Cheat Engine)
     *      -> Сканируем из диска в диск            (Cheat Engine)
     */



    //int isd = 0;
    //for(RE::value_t val : *RE::globals->scans.first) {
    //    cout<<"val: "<<isd<<": "<<val<<endl;
    //    isd++;
    //}
    return 0;
}
