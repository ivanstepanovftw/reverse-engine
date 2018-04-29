//
// Функции, которым не нашлось место
//

#ifndef RE_EXTERNAL_HH
#define RE_EXTERNAL_HH

#include <iosfwd>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

using namespace std;

vector<string> split(const string &text, const string &delims, uint64_t d = UINT64_MAX);

string execute(string cmd);

vector<vector<string>> getProcesses();

size_t get_mem_total(size_t i = 0);

// fixme ну и костыль...
size_t get_mem_free();


#endif //RE_EXTERNAL_HH
