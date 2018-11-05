/*
    This file is part of Reverse Engine.

    External tools

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This library is free software: you can redistribute it and/or modify
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

#include <string>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdint>
#include <limits>


namespace RE {

/** 
 * Retrieve a summary of tokens, no more than max_tokens size.
 * 
 * @example:
 *   @arg text: "  \t This is   a very long string"
 *   @arg delims: "\t "
 *   @arg max_tokens_count: 3
 *   @return { "This", "is", "a very long string" }
 */
static
std::vector<std::string>
tokenize(const std::string& text,
         const std::string& delims,
         size_t max_tokens = SIZE_MAX)
{
    using namespace std;
    vector<std::string> tokens;
    size_t _start = text.find_first_not_of(delims),
             _end = 0;
    
    for(; (_end = text.find_first_of(delims, _start)) != std::string::npos && max_tokens > 1; max_tokens--) {
        tokens.push_back(text.substr(_start, _end - _start));
        _start = text.find_first_not_of(delims, _end);
    }
    // last token remain
    if (_start != std::string::npos)
        tokens.push_back(text.substr(_start));
    
    return tokens;
}

/** 
 * Execute command, return output
 */
static
std::string
execute(const std::string& cmd)
{
    char buffer[256];
    std::string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe))
            if (fgets(buffer, 256, pipe) != nullptr)
                result += buffer;
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


struct CProcess {
    std::string pid;
    std::string user;
    std::string command;

    std::string str() {
        return "["+pid+", "+user+", "+command+"]";
    }
};


static
std::vector<CProcess>
getProcesses()
{
    std::string executed = execute("ps -wweo pid=,user=,command=-sort=-pid");
    std::size_t lines_to_skip = 1;  // skip N first lines
    std::vector<std::string> rows = tokenize(executed, "\n");
    std::vector<CProcess> processes;
    
    for(const std::string& row : rows) {
        if (lines_to_skip > 0) {
            lines_to_skip--;
            continue;
        }

        std::vector<std::string> to_push = tokenize(row, " ", 3);
        if (to_push.size() < 3)
            continue;
        
        processes.emplace_back(CProcess {to_push[0], to_push[1], to_push[2]});
    }
    return processes;
}


static
std::size_t
get_mem_total(std::size_t i = 0)
{
    std::string ss;
    switch (i) {
        case 1: ss = "MemFree:";      break;
        case 2: ss = "MemAvailable:"; break;
        case 3: ss = "Cached:";       break;
        default:ss = "MemTotal:";
    }
    static std::string token;
    std::ifstream file("/proc/meminfo");
    while (file>>token) {
        if (token == ss) {
            unsigned long mem;
            if (file>>mem) {
                return mem * 1024;
            } else {
                return 0;
            }
        }
        // ignore rest of the line
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return 0; // nothing found
}

// fixme ну и костыль xD
static
std::size_t
get_mem_free()
{
    return get_mem_total(2) + get_mem_total(3);
}

} //namespace RE
