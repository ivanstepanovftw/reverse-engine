#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <memory>

#include <r_core.h>
#include <r_types.h>
#include <r_util.h>
#include <r_bin.h>
#include <r_io.h>
#include <regex>
#include <cmath>


namespace sfs = std::filesystem;
// namespace bio = boost::iostreams;

class Timer {
public:
    explicit Timer(std::string what = "Timer")
            : m_what(std::move(what)), m_tp(std::chrono::high_resolution_clock::now()) {}

    ~Timer() {
        std::clog << m_what << ": done in " << std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - m_tp).count() << " seconds" << std::endl;
    }

private:
    std::string m_what;
    std::chrono::high_resolution_clock::time_point m_tp;
};




void foo(std::istream& is, std::ostream& os) {
    std::string s;
    while (getline(is, s, '\n')) {
        os << s;
    }
}

int main(int argc, char *const argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    foo(std::cin, std::cout);

    return 0;
}