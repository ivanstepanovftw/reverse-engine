/*
    This file is part of Reverse Engine.

    Simple trainer example.

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <iomanip>
#include <chrono>
#include <type_traits>
#include <zconf.h>
#include <functional>
#include <cmath>
#include <cassert>
#include <reverseengine/core.hh>
#include <reverseengine/value.hh>
#include <reverseengine/scanner.hh>


using namespace std;
using namespace std::chrono;
using namespace std::literals;


//const string target = "FakeGame";
//const string target = "FakeMem";
const string target = "csgo_linux64";
//const string target = "dota2";
//const pid_t target = 1337;

int
main() 
{
    if (getuid() > 0) {
        cout<<"Warning: running without root"<<endl;
    }
    /// Trainer and scanner example
    RE::Handle h;
    RE::Cregion *exe = nullptr;
    RE::Cregion *libc = nullptr;
    RE::Cregion *ld = nullptr;
    
stage_waiting:;
    cout<<"Waiting for '"<<target<<"' process"<<endl;
    for(;;) {
        h.attach(target);
        if (h.is_good())
            break;
        usleep(500'000);
    }
    cout<<"Found! PID: "<<h.pid<<", title: "<<h.title<<endl;
    cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    
stage_updating:;
    for(;;) {
        h.update_regions();
        exe = h.get_region_by_name(h.title);
        libc = h.get_region_by_name("libc-2.26.so");
        ld = h.get_region_by_name("ld-2.26.so");
        if (exe && libc && ld)
            break;
        if (!h.is_running())
            goto stage_waiting;
        usleep(500'000);
    }
    cout<<"Regions added: "<<h.regions.size()<<", ignored: "<<h.regions_ignored.size()<<endl;
    cout<<"Found region: "<<exe->pathname<<endl;
    cout<<"Found region: "<<libc->pathname<<endl;
    cout<<"Found->>>region: "<<ld->pathname<<endl;
    cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    
    const uintptr_t test_address = exe->address+16;
    RE::Cregion *test_region = h.get_region_of_address(test_address);
    if (!test_region) {
        cout<<"Can't find region of address: address: "<<RE::HEX(test_address)<<endl;
        return 1;
    }
    cout<<"Address: "<<RE::HEX(test_address)<<" belongs to "<<test_region->pathname<<endl;
    cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    
    cout<<"Lets read 8 bytes to our union from our target address-space: "<<RE::HEX(test_address)<<endl;
    RE::mem64_t test_memory;  /// it can be any type you need
    errno = 0;
    if (h.read(test_address, &test_memory, sizeof(test_memory)) != sizeof(test_memory)) {
        if (errno) {
            cout<<"Error while reading: "<<strerror(errno)<<endl;
            return 1;
        } else
            cout<<"We have read partially"<<endl;
    }
    cout<<"We can easily reinterpret this in other types: "<<endl;
    cout<<"float: "<<test_memory.f32<<endl;
    cout<<"double: "<<test_memory.f64<<endl;
    cout<<"4 bytes unsigned: "<<test_memory.u32<<endl;
    cout<<"8 bytes signed: "<<test_memory.i32<<endl;
    cout<<"8 chars as text: '"<<+test_memory.chars<<"'"<<endl;
    cout<<"8 chars in hex: "<<RE::HEX(*reinterpret_cast<uint64_t *>(test_memory.bytes))<<endl;
    cout<<"8 bytes in dec: ";
    cout<<hex<<showbase;
    ostream_iterator<int> out_it(cout, ", ");
    copy(test_memory.chars, test_memory.chars + sizeof(RE::mem64_t), out_it);
    cout<<dec<<endl;
    cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    
    uintptr_t found;
    if (!h.find_pattern(&found,
                       ld, 
                       "\x48\x89\xC7\xE8\x00\x00\x00\x00", 
                       "xxxx????")) {
        clog<<"Not found :("<<endl;
    } else
        clog<<"found: "<<RE::HEX(found)<<endl;
    
    return 0;
}
