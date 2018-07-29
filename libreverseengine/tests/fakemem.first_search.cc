//
// Created by root on 28.07.18.
//

#include <GUI/globals.hh>
#include <iostream>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {

    char *data;
    delete[] data;


    string target = "FakeMem";
    string search_for = "0";

    globals.handle = new Handle(target);
    globals.handle->update_regions();
    globals.scanner = new Scanner(globals.handle);


    clog<<"FakeMem, pid: "<<globals.handle->pid<<endl;

    high_resolution_clock::time_point timestamp;

    scan_data_type_t data_type = ANYNUMBER;
    uservalue_t uservalue[2];
    scan_match_type_t match_type;
    try {
        globals.scanner->string_to_uservalue(data_type, search_for, &match_type, uservalue);
    } catch (bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        //return;
        return 0;
    }

    globals.scans.last = new matches_t();
    globals.scans.first = new matches_t();


    timestamp = high_resolution_clock::now();
    globals.scanner->scan(*globals.scans.last, data_type, uservalue, match_type);
    clog<<"Scan 1/2 done in: "
        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    assert(globals.scans.last->size() == globals.scans.last->matches_size);
    clog<<"swaths_count: "<<globals.scans.last->swaths_count<<endl;
    clog<<"swaths_allocated: "<<globals.scans.last->swaths_allocated<<endl;
    clog<<"size: "<<globals.scans.last->size()<<endl;
    clog<<"virt: "<<globals.scans.last->virt()<<endl;
    clog<<"allo: "<<globals.scans.last->allo()<<endl;

//    timestamp = high_resolution_clock::now();
//    globals.scanner->scan_next(*globals.scans.last, *globals.scans.first, data_type, uservalue, match_type);
//    clog<<"Scan 2/2 done in: "
//        <<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
//        <<" seconds"<<endl;
//
//    clog<<"swaths_count: "<<globals.scans.first->swaths_count<<endl;
//    clog<<"swaths_allocated: "<<globals.scans.first->swaths_allocated<<endl;
//    clog<<"size: "<<globals.scans.first->size()<<endl;
//    clog<<"virt: "<<globals.scans.first->virt()<<endl;
//    clog<<"allo: "<<globals.scans.first->allo()<<endl;

    return 0;
}
