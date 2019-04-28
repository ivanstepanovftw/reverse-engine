#include <iostream>
#include <sstream>
#include <zconf.h>



int main(int argc, char *argv[]) {
    using namespace std;

    cout<<"pid: "<<getpid()<<endl;
    cout<<"ppid: "<<getppid()<<endl;
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
            return code;
        }
    }
    else if (argc >= 2) {
        cout<<"argv[1]: "<<argv[0]<<endl;
        uint64_t code;
        istringstream iss(argv[1]);
        iss >> code;
        return code;
    }

    return 0;
}
