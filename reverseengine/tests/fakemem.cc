/*
    This file is part of Reverse Engine.

    

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

#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cstdint>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <sstream>
#include <strings.h>

#define HEX(s) hex<<showbase<<(s)<<dec

using namespace std;

ssize_t mib;
char *array = nullptr;
bool is_random;

void
print_help() {
    cout<<"---------------------------------"<<endl;
    cout<<" Command  | Description          "<<endl;
    cout<<"----------+----------------------"<<endl;
    cout<<" > N      | reallocate N bytes   "<<endl;
    cout<<" > -N     | reallocate N MiB     "<<endl;
    cout<<" > del    | delete an array      "<<endl;
    cout<<" > info   | prints info          "<<endl;
    cout<<" > rand   | turn on/off random   "<<endl;
    cout<<"---------------------------------"<<endl;
}

ssize_t m2b (ssize_t m) {
    if (m < 0) 
        return - m * 1024 * 1024 / sizeof(char);
    else
        return m;
}

void
print_info() {
    cout<<"allocated? "<<(bool)array<<endl;
    if (array) {
        cout<<"allocated: "<<mib<<" MiB == "<<m2b(mib)<<" bytes"<<endl;
    }
    cout<<"is_random: "<<(bool)is_random<<endl;
}

void
reallocate() {
//fixme govnocod
    delete[] array;
    ssize_t to_allocate = m2b(mib);
    cout<<"to_allocate: "<<to_allocate<<" bytes"<<endl;
    array = new char[to_allocate];
    
    if (is_random) {
        for(ssize_t i = 0; i < m2b(mib); i++)
            array[i] = static_cast<char>(rand());
    } else
        bzero(array, m2b(mib));
}


int
main(int argc, char **argv) {
    is_random = false;
    mib = 10;
    reallocate();
    
    print_help();
    
    string command;
    while(true) {
        cout<<"> ";
        cin>>command;
        
        if (command.substr(0, 1) == "d") {  // delete
            if (array) {
                delete[] array;
                array = nullptr;
                cout<<"Array deleted. Freed memory: "<<mib<<" MiB."<<endl;
                mib = 0;
            } else
                cout<<"Already deleted."<<endl;
            continue;
        }
        else if (command.substr(0, 1) == "i") {  // info
            print_info();
            continue;
        }
        else if (command.substr(0, 1) == "r") {  // switch random
            is_random = !is_random;
            cout<<"is_random: "<<(bool)is_random<<endl;
            continue;
        }
        else {
            istringstream iss(command);
            iss>>mib;
            if (iss.fail()) {
                cout<<"Please enter an integer."<<endl;
                continue;
            }
            reallocate();
        }
    }
}