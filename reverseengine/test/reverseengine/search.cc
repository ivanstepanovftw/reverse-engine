//
// Created by root on 28.07.18.
//

#include <reverseengine/globals.hh>
#include <iostream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    string target = "FAKEMEM";
    string search_for = "0";

    globals.handle = new RE::Handle(target);
    if (!globals.handle->is_good())
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");
    globals.handle->update_regions();
    globals.scanner = new RE::Scanner(globals.handle);


    clog<<"FAKEMEM, pid: "
        <<globals.handle->pid
        <<", title: "<<globals.handle->title
        <<endl;

    high_resolution_clock::time_point timestamp;

//    RE::Edata_type data_type = RE::Edata_type::ANYNUMBER;
    RE::Edata_type data_type = RE::Edata_type::INTEGER8;
    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        globals.scanner->string_to_uservalue(data_type, search_for, &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        //return;
        return 0;
    }

    globals.scans.first = new RE::matches_t();
    globals.scans.prev = new RE::matches_t();
    globals.scans.last = new RE::matches_t();

    timestamp = high_resolution_clock::now();
    globals.scanner->scan(*globals.scans.first, data_type, uservalue, match_type);
    clog<<"Scan 1/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;
    assert(globals.scans.first->size() == globals.scans.first->matches_size);
    clog<<"size: {counted: "<<globals.scans.first->size()
            <<", matches_size: "<<globals.scans.first->matches_size<<"}"<<endl;
//    clog<<"mem_virt: "<<globals.scans.first->mem_virt()<<endl;
//    clog<<"mem_allo: "<<globals.scans.first->mem_allo()<<endl;
//    clog<<"swaths.size: "<<globals.scans.first->swaths.size()<<endl;
//    clog<<"swaths.capacity: "<<globals.scans.first->swaths.capacity()<<endl;

    timestamp = high_resolution_clock::now();
    globals.scanner->scan_next(*globals.scans.first, *globals.scans.first, data_type, uservalue, match_type);
    clog<<"Scan 2/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    assert(globals.scans.first->size() == globals.scans.first->matches_size);
    clog<<"size: {counted: "<<globals.scans.first->size()
            <<", matches_size: "<<globals.scans.first->matches_size<<"}"<<endl;
//    clog<<"mem_virt: "<<globals.scans.last->mem_virt()<<endl;
//    clog<<"mem_allo: "<<globals.scans.last->mem_allo()<<endl;
//    clog<<"swaths.size: "<<globals.scans.last->swaths.size()<<endl;
//    clog<<"swaths.capacity: "<<globals.scans.last->swaths.capacity()<<endl;

    timestamp = high_resolution_clock::now();
    globals.scanner->scan_next(*globals.scans.first, *globals.scans.first, data_type, uservalue, match_type);
    //TODO[med]: (TO INVESTIGATE) really interesting result (why second scan is faster?)
    clog<<"Scan 3/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    assert(globals.scans.first->size() == globals.scans.first->matches_size);
    clog<<"size: {counted: "<<globals.scans.first->size()
            <<", matches_size: "<<globals.scans.first->matches_size<<"}"<<endl;
//    clog<<"mem_virt: "<<globals.scans.last->mem_virt()<<endl;
//    clog<<"mem_allo: "<<globals.scans.last->mem_allo()<<endl;
//    clog<<"swaths.size: "<<globals.scans.last->swaths.size()<<endl;
//    clog<<"swaths.capacity: "<<globals.scans.last->swaths.capacity()<<endl;

    return 0;
}
