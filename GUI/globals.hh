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
#ifndef RE_GLOBALS_HEADER
#define RE_GLOBALS_HEADER

#include <libreverseengine/core.hh>
#include <libreverseengine/value.hh>
#include <libreverseengine/scanner.hh>


// todo[low]: remove namespace
using namespace std;
using namespace std::chrono;    


constexpr size_t REFRESH_RATE = 2000;


class scans_t {
public:
    RE::matches_t *first = nullptr;
    RE::matches_t *prev  = nullptr;
    RE::matches_t *last  = nullptr;
    
    void scan_new() {
        delete first;
        delete prev;
        delete last;
    }
    
private:
    
};


// fixme [med]: or use it as namespace?
struct {
    RE::Handle *handle = nullptr;  //fixme [low]: remove pointer
    RE::Scanner *scanner = nullptr;  //fixme [low]: remove pointer
    scans_t scans;
} globals;

#endif //RE_GLOBALS_HEADER
