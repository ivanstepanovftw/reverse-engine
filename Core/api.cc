//
// Created by root on 16.02.18.
//

#include "api.hh"

using namespace std;

//https://stackoverflow.com/a/7408245
//https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
vector<string>
split(const string& text, const string& delims, uint64_t d) {
    vector<string> tokens;
    size_t start = text.find_first_not_of(delims), end = 0;

    for(;(end = text.find_first_of(delims, start)) != string::npos && d>1; d--) {
        tokens.push_back(text.substr(start, end - start));
        start = text.find_first_not_of(delims, end);
    }
    if (start != string::npos)
        tokens.push_back(text.substr(start));

    return tokens;
}

string
execute(string cmd) {
    char buffer[128];
    string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw runtime_error("popen() failed!");
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

vector<vector<string>> getProcesses() {
    string executed = execute("ps -wweo pid=,user=,command=-sort=-pid");
    int lines_to_skip = 1;
    vector<string> rows = split(executed, "\n");
    vector<vector<string>> processes;
    processes.clear();
    for (const string &row : rows) {
        if (lines_to_skip-- > 0)
            continue;

        vector<string> to_push;
        for (const auto &col : split(row, " ", 3))
            to_push.push_back(col);

        processes.push_back(to_push);
    }
    return processes;
}
