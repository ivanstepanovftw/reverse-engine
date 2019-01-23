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
    string search_for = "145";

    Singleton::getInstance()->handle = new RE::Handle(target);
    if (!Singleton::getInstance()->handle->is_good())
        throw std::invalid_argument("Cannot find "+target+" process. Nothing to do.");
    Singleton::getInstance()->handle->update_regions();
    Singleton::getInstance()->scanner = new RE::Scanner(Singleton::getInstance()->handle);


    clog<<"FAKEMEM, pid: "
        <<Singleton::getInstance()->handle->pid
        <<", title: "<<Singleton::getInstance()->handle->title
        <<endl;

    high_resolution_clock::time_point timestamp;

    //RE::Edata_type data_type = RE::Edata_type::INTEGER32;
    RE::Edata_type data_type = RE::Edata_type::ANYNUMBER;
    //RE::Edata_type data_type = RE::Edata_type::INTEGER8;
    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        Singleton::getInstance()->scanner->string_to_uservalue(data_type, search_for, &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        //return;
        return 0;
    }

    Singleton::getInstance()->scans.first = new RE::matches_t();

    timestamp = high_resolution_clock::now();
    Singleton::getInstance()->scanner->scan_regions(*Singleton::getInstance()->scans.first, data_type, uservalue, match_type);
    clog<<"Scan 1/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;
    assert(Singleton::getInstance()->scans.first->size() == Singleton::getInstance()->scans.first->matches_size);
    clog<<"size: {counted: "<<Singleton::getInstance()->scans.first->count()<<"}"<<endl;
    //clog<<"mem_virt: "<<Singleton::getInstance()->scans.first->mem_virt()<<endl;
    //clog<<"mem_allo: "<<Singleton::getInstance()->scans.first->mem_allo()<<endl;
    //clog<<"swaths.size: "<<Singleton::getInstance()->scans.first->swaths_count<<endl;
    //clog<<"swaths.capacity: "<<Singleton::getInstance()->scans.first->swaths_allocated<<endl;

    timestamp = high_resolution_clock::now();
    Singleton::getInstance()->scanner->scan_update(*Singleton::getInstance()->scans.first);
    Singleton::getInstance()->scanner->scan_recheck(*Singleton::getInstance()->scans.first, data_type, uservalue, match_type);
    clog<<"Scan 2/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    clog<<"size: {counted: "<<Singleton::getInstance()->scans.first->count()<<"}"<<endl;
    //clog<<"swaths.size: "<<Singleton::getInstance()->scans.first->swaths_count<<endl;
    //clog<<"swaths.capacity: "<<Singleton::getInstance()->scans.first->swaths_allocated<<endl;

    timestamp = high_resolution_clock::now();
    Singleton::getInstance()->scanner->scan_update(*Singleton::getInstance()->scans.first);
    Singleton::getInstance()->scanner->scan_recheck(*Singleton::getInstance()->scans.first, data_type, uservalue, match_type);
    //TODO[med]: (TO INVESTIGATE) really interesting result (why second scan is faster?)
    clog<<"Scan 3/3 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    clog<<"size: {counted: "<<Singleton::getInstance()->scans.first->count()<<"}"<<endl;
    //clog<<"swaths.size: "<<Singleton::getInstance()->scans.first->swaths_count<<endl;
    //clog<<"swaths.capacity: "<<Singleton::getInstance()->scans.first->swaths_allocated<<endl;

    int isd = 0;
    for(RE::value_t val : *Singleton::getInstance()->scans.first) {
        cout<<"val: "<<isd<<": "<<val<<endl;
        isd++;
    }

    clog<<"size: "<<Singleton::getInstance()->scans.first->size()<<endl;
    clog<<"count: "<<Singleton::getInstance()->scans.first->count()<<endl;

    //for (auto&& s : Singleton::getInstance()->scans.first->swaths) {
    //    for (auto && b : s.bytes) {
    //        ;
    //    }
    //}

    return 0;
}
