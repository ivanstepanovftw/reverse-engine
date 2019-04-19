#include <iostream>
#include <sstream>
#include <zconf.h>


void do_thing(uint64_t code) {
    using namespace std;

    cout<<"code: "<<code<<endl;


    if (code == 0)
        return;
    if (code == 6) {
        //SIGABRT

    }
    if (code == 11) {
        //SIGSEGV
        char *c = nullptr;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        *c = 1;
#pragma clang diagnostic pop
    }


}

int main(int argc, char *argv[]) {
    using namespace std;

    cout<<"pid: "<<getpid()<<endl;
    cout<<"argv[0]: "<<argv[0]<<endl;

    if (argc == 1) {
        while (true) {
            uint64_t code;
            cin >> code;
            if (cin.fail()) {
                cout << "Please enter an integer" << endl;
                cin.clear(); // clear error state
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard 'bad' character(s)
                continue;
            }
            do_thing(code);
        }
    }
    else if (argc >= 2) {
        cout<<"argv[1]: "<<argv[0]<<endl;
        uint64_t code;
        istringstream iss(argv[1]);
        iss >> code;
        do_thing(code);
    }

    return 0;
}
