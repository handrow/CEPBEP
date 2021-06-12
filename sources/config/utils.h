#ifndef SOURCES_CONFIG_UTILS_H_
# define SOURCES_CONFIG_UTILS_H_

#include <string>
#include <vector>

namespace Config {

std::vector<std::string> stringSplit(std::string str, char sign);
void trim(std::string &str, char sign);
std::string EditLine(std::string line, std::ifstream& file);
std::vector<std::string> FileReading(const std::string& filepath, Error *err);

};  // namespace Config

#endif  // SOURCES_CONFIG_UTILS_H_
