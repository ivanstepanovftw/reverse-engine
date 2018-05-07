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

#ifndef RE_EXTERNAL_HH
#define RE_EXTERNAL_HH

#include <string>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdint>

std::vector<std::string>
split(const std::string& text,
      const std::string& delims,
      uint64_t d = std::numeric_limits<uint64_t>::max())
{
    using namespace std;
    //https://stackoverflow.com/a/7408245
    //https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
    vector<string> tokens;
    size_t start = text.find_first_not_of(delims), _end = 0;
    for(; (_end = text.find_first_of(delims, start)) != string::npos && d > 1; d--) {
        tokens.push_back(text.substr(start, _end - start));
        start = text.find_first_not_of(delims, _end);
    }
    if (start != string::npos)
        tokens.push_back(text.substr(start));
    
    return tokens;
}

std::string
execute(const std::string& cmd)
{
    using namespace std;
    static char buffer[128];
    static string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw runtime_error("popen() failed!");
    try {
        while (!feof(pipe))
            if (fgets(buffer, 128, pipe) != nullptr)
                result += buffer;
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


std::vector<std::vector<std::string>>
getProcesses()
{
    using namespace std;
    string executed = execute("ps -wweo pid=,user=,command=-sort=-pid");
    int lines_to_skip = 1;
    vector<string> rows = split(executed, "\n");
    vector<vector<string>> processes;
    processes.clear();
    for(const string& row : rows) {
        if (lines_to_skip-- > 0)
            continue;
        
        vector<string> to_push;
        for(const auto& col : split(row, " ", 3))
            to_push.push_back(col);
        
        processes.push_back(to_push);
    }
    return processes;
}


size_t
get_mem_total(size_t i)
{
    using namespace std;
    static string ss;
    switch (i) {
        case 1: ss = "MemFree:";      break;
        case 2: ss = "MemAvailable:"; break;
        case 3: ss = "Cached:";       break;
        default:ss = "MemTotal:";
    }
    static string token;
    ifstream file("/proc/meminfo");
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
        file.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return 0; // nothing found
}

// fixme ну и костыль xD
size_t
get_mem_free()
{
    return get_mem_total(2) + get_mem_total(3);
}

#endif //RE_EXTERNAL_HH
