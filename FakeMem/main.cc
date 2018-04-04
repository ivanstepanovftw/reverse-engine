//
// Created by root on 05.02.18.
//
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

size_t mib;
char *array = nullptr;
bool is_random;

void
print_help() {
    cout<<"---------------------------------"<<endl;
    cout<<" Command  | Description          "<<endl;
    cout<<"----------+----------------------"<<endl;
    cout<<" > N      | allocate N mebibytes "<<endl;
    cout<<" > del    | delete an array      "<<endl;
    cout<<" > info   | prints info          "<<endl;
    cout<<" > rand   | turn on/off random   "<<endl;
    cout<<"---------------------------------"<<endl;
}

size_t m2b (size_t m) {
    return m * 1024 * 1024 / sizeof(char);
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
    if (array)
        delete [] array;
    size_t to_allocate = m2b(mib);
    cout<<"to_allocate: "<<to_allocate<<" bytes"<<endl;
    array = new char[to_allocate];
    
    if (is_random) {
        for(size_t i = 0; i < m2b(mib); i++)
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