#ifndef CONFIG_PARSER_H_
# define CONFIG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdio>

std::vector<std::string> FileReading(std::string file_name);
std::map<std::string, std::map<std::string, std::string> > Parser(std::vector<std::string> data);
std::map<std::string, std::string> ParserCategory(std::vector<std::string>, size_t start);

#endif