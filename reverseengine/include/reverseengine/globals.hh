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
#pragma once

#include <reverseengine/common.hh>
#include <reverseengine/core.hh>
#include <reverseengine/value.hh>
#include <reverseengine/scanner.hh>

NAMESPACE_BEGIN(RE)

constexpr size_t REFRESH_RATE = 2000;

class CScans {
public:
    RE::matches_t *first = nullptr;
    RE::matches_t *prev  = nullptr;
    RE::matches_t *last  = nullptr;

    void scan_new() {
        delete first;
        delete prev;
        delete last;
    }

    void scan_first() {
        last = new RE::matches_t();
        first = last;
    }

    void scan_next() {
        delete prev;
        prev = last;
        last = new RE::matches_t();
    }

    void scan_undo() {
        delete last;
        last = prev;
    }
};


//class Singleton
//{
//private:
//    /* Private constructor to prevent instancing. */
//    Singleton() {};
//
//public:
//    /* Static access method. */
//    static Singleton* getInstance() {
//        static Singleton instance;
//        return &instance;
//    }
//
//    RE::handler *handler;
//    RE::Scanner *scanner;
//    CScans scans;
//};


class globals_t {
public:
    RE::Process *handle;
    RE::Scanner *scanner;
    CScans scans;
};

extern std::shared_ptr<globals_t> globals;

NAMESPACE_END(RE)
