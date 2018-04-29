//
// Created by root on 11.03.18.
//

#include "external.hh"


std::vector<std::string>
split(const std::string &text, const std::string &delims, uint64_t d)
{
    //https://stackoverflow.com/a/7408245
    //https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
    std::vector<std::string> tokens;
    size_t start = text.find_first_not_of(delims), end = 0;
    
    for(; (end = text.find_first_of(delims, start)) != std::string::npos && d>1; d--) {
        tokens.push_back(text.substr(start, end - start));
        start = text.find_first_not_of(delims, end);
    }
    if (start != std::string::npos)
        tokens.push_back(text.substr(start));
    
    return tokens;
}


std::string
execute(std::string cmd)
{
    char buffer[128];
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != nullptr)
                result += buffer;
        }
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
    std::string executed = execute("ps -wweo pid=,user=,command=-sort=-pid");
    int lines_to_skip = 1;
    std::vector<std::string> rows = split(executed, "\n");
    std::vector<std::vector<std::string>> processes;
    processes.clear();
    for (const std::string &row : rows) {
        if (lines_to_skip-- > 0)
            continue;
        
        std::vector<std::string> to_push;
        for (const auto &col : split(row, " ", 3))
            to_push.push_back(col);
        
        processes.push_back(to_push);
    }
    return processes;
}

size_t
get_mem_total(size_t i) {
    string ss;
    switch (i) {
        case 1: ss="MemFree:"; break;
        case 2: ss="MemAvailable:"; break;
        case 3: ss="Cached:"; break;
        default:ss="MemTotal:";
    }
    string token;
    ifstream file("/proc/meminfo");
    while(file >> token) {
        if(token == ss) {
            unsigned long mem;
            if(file >> mem) {
                return mem*1024;
            } else {
                return 0;
            }
        }
        // ignore rest of the line
        file.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return 0; // nothing found
}

size_t
get_mem_free() {
    return get_mem_total(2)+get_mem_total(3);
}
