#ifndef SOURCES_CONFIG_UTILS_H_
# define SOURCES_CONFIG_UTILS_H_

#include <string>
#include <vector>
#include <fstream>

namespace Config {

std::vector<std::string> string_split(const std::string& str, char sign);
std::string trim(const std::string& str, char delimiter);

};  // namespace Config

#endif  // SOURCES_CONFIG_UTILS_H_
