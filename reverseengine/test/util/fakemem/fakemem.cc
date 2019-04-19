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
#include <random>
#include <strings.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <iomanip>
#include <functional>
#include <exception>          // std::bad_alloc
#include <csignal>
#include <csetjmp>
#include <fstream>


static
std::string
help() {
    //todo[med]: uglycode: try raw literal
    std::ostringstream result;
    result << "---------------------------------" << "\n"
           << " Command  | Description          " << "\n"
           << "----------+----------------------" << "\n"
           << " > N      | reallocate N bytes" << "\n"
           << " > -N     | reallocate N MiB" << "\n"
           << " > del    | delete an array" << "\n"
           << " > info   | prints info" << "\n"
           << " > flat   | flat values" << "\n"
           << " > seq    | sequential values" << "\n"
           << " > rand   | random values" << "\n"
           << "---------------------------------";
    return result.str();
}


/// Random generator
template <typename _Type>
class CRandomGenerator {
public:
    CRandomGenerator() {
        srand(static_cast<unsigned>(time(nullptr)));
    }

    [[gnu::always_inline]]
    _Type
    operator()() {
        // return (static_cast<_Type>(rand()) % std::numeric_limits<_Type>::max()) + std::numeric_limits<_Type>::min(); // fixme[low]: (RAND LIMITATIONS)
        return static_cast<_Type>(rand() - RAND_MAX/2);
    }
};

/// Sequential generator
template <typename _Type>
class CSequentialGenerator {
public:
    CSequentialGenerator() {
        this->c = std::numeric_limits<_Type>::max();
    }

    [[gnu::always_inline]]
    _Type
    operator()() {
        if (c++ == std::numeric_limits<_Type>::max())
            c = std::numeric_limits<_Type>::min();
        return c;
    }

private:
    _Type c;
};

template <typename _Type>
class Heap {
public:
    std::vector<_Type> array;

    Heap() = default;

    template <class _Func>
    bool
    generate_data(size_t size, _Func f)
    {
        generator = default_generator;
        try {
            array.resize(size/sizeof(_Type));
        } catch (const std::bad_alloc& e) {
            std::cerr<<"Too much memory requested"<<std::endl;
            array.clear();
            array.shrink_to_fit();
            return false;
        }
        array.shrink_to_fit();
        generator = typeid(f).name(); //fixme[low]: idk another way to get function name
        std::generate(array.begin(), array.end(), f);
        return true;
    }

    static
    std::string
    humanReadableByteCount(size_t bytes, bool si = false)
    {
        using namespace std::string_literals;
        size_t unit = si ? 1000 : 1024;
        if (bytes < unit)
            return std::string(std::to_string(bytes) + " B"s);
        size_t exp = static_cast<size_t>(log(bytes) / log(unit));
        std::string pre = (si ? "kMGTPE"s : "KMGTPE"s)[exp-1] + (si ? ""s : "i"s);
        char *ret; // fixme[low]: are you kidding?
        asprintf(&ret, "%.1f %sB", bytes / pow(unit, exp), pre.c_str());
        return std::string(ret);
    }

    std::string
    str()
    {
        constexpr size_t k = 16;
        size_t v = std::min(array.size(), k);
        size_t b = std::min(array.size() * sizeof(_Type), k);
        std::ostringstream values;
        std::ostringstream bytes;
        for (size_t i = 0; i < v; i++) {
            values << +array[i];
            if (i != v - 1)
                values << ", ";
        }
        bytes << std::hex << std::noshowbase << std::setfill('0');
        for (uint8_t *cur = reinterpret_cast<uint8_t *>(&array[0]); cur < reinterpret_cast<uint8_t *>(&array[0]) + b; cur++) {
            bytes << "\\x" << std::setw(2) << +*cur;
        }

        std::ostringstream result;
        result << "size: " << this->humanReadableByteCount(array.size() * sizeof(_Type)) << " (" << array.size() * sizeof(_Type) << ")" << "\n"
               << "length: " << array.size() << "\n"
               << "with generator: " << generator << "\n"
               << "address: " << reinterpret_cast<void *>(&array[0]) << "\n"
               << "first " << v << " values: ["<<values.str()<< "]" << "\n"
               << "first " << b << " bytes: r'"<<bytes.str()<<"'";
        return result.str();
    }

    const char *default_generator = "<unknown>";
    std::string generator = default_generator;
};


int
main(int argc, char *argv[])
{
    using std::clog, std::cout, std::cerr, std::flush, std::endl, std::cin;
    using _Type = unsigned int; // fixme[low]: (RAND LIMITATIONS) will use 'int' instead of 'uint64_t'

    { std::ofstream f("/proc/self/oom_score_adj", std::ios::out | std::ios::binary); if (f.good()) f<<"1000"; }
    CSequentialGenerator<_Type> s;
    CRandomGenerator<_Type> r;

    Heap<_Type> h;

    enum { // fixme[low]: ugly code
        FLAT,
        SEQUENTIAL,
        RANDOM
    } generator = FLAT;

    cout<<help()<<endl;
    while (true) {
        cout << "> " << flush;
        std::string command;
        cin >> command;
        if        (command.substr(0, 1) == "d") { h.generate_data(0, []() { return 0; }); continue;
        } else if (command.substr(0, 1) == "i") { cout<<h.str()<<endl; continue;
        } else if (command.substr(0, 1) == "f") { generator = FLAT; continue;
        } else if (command.substr(0, 1) == "s") { generator = SEQUENTIAL; continue;
        } else if (command.substr(0, 1) == "r") { generator = RANDOM; continue;
        } else if (command.substr(0, 1) == "q") { return 0;
        } else {
            ssize_t allocate_please = 0;
            std::istringstream iss(command);
            iss >> allocate_please;
            if (iss.fail()) {
                cout << "Please enter an integer." << endl;
                continue;
            }
            size_t allocate_that = 0;
            if (allocate_please < 0)
                allocate_that = static_cast<size_t>(-allocate_please * (1u<<20u));
            else
                allocate_that = static_cast<size_t>(allocate_please);
            switch (generator) {
                case FLAT:       h.generate_data(allocate_that, []() { return 0; }); break;
                case SEQUENTIAL: h.generate_data(allocate_that, s); break;
                case RANDOM:     h.generate_data(allocate_that, r); break;
            }
        }
    }
}
