//
// Created by root on 21.04.19.
//

#include <iostream>
#include <sstream>
#include <zconf.h>

int main(int argc, char *argv[]) {
    using namespace std;

    cout<<"pid: "<<getpid()<<endl;
    cout<<"ppid: "<<getppid()<<endl;
    cout<<"argv[0]: "<<argv[0]<<endl;

    char *x = new char;
    char *y = x;
    delete x;
    delete y;

    return 0;
}
