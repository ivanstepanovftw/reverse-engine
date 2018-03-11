//
// Функции, которым не нашлось место
//

#ifndef RE_EXTERNAL_HH
#define RE_EXTERNAL_HH

#include <string>
#include <vector>
#include <stdexcept>


std::vector<std::string> split(const std::string &text, const std::string &delims, uint64_t d = UINT64_MAX);

std::string execute(std::string cmd);

std::vector<std::vector<std::string>> getProcesses();

#endif //RE_EXTERNAL_HH
