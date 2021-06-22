#include "common/types.h"
#include "common/error.h"
#include "config/config.h"
#include "config/utils.h"

#include <cstdio>
#include <iostream>
#include <string>

// std::vector<std::string> pars_regul(std::string regul) {
//     std::vector<std::string> vector_regul;
//     std::string added_element;
//     for (usize i = 0; i < regul.size(); i++) {
//         if (regul[i] == '*' || regul[i] == '+')
//             vector_regul.push_back(added_element = regul[i]);
//         else {
//             for (; i < regul.size() && regul[i] != '*' && regul[i] != '+'; i++) {
//                 if (regul[i] == '{' || regul[i] == '}') {
//                     i++;
//                     break;
//                 }
//                 if (regul[i] == '\\' && i + 1 < regul.size())
//                     i++;
//                 added_element.push_back(regul[i]);
//             }
//             vector_regul.push_back(added_element);
//             i--;
//         }
//         added_element.clear();
//     }
//     return vector_regul;
// }

// usize check_word(const std::string& str, const std::string& word, usize start) {
//     if (word.find("|") > word.size())
//         return str.find(word, start) + word.size();
//     std::vector<std::string> check = Config::string_split(word, '|');
//     usize count_return;
//     for (usize i = 0; i < check.size(); i++) {
//         count_return = str.find(check[i], start);
//         if (count_return < str.size())
//             return count_return + check[i].size();
//     }
//     return str.max_size();
// }

// bool regular_expression(std::string regul, std::string str) {
//     std::vector<std::string> vector_regul = pars_regul(regul);
//     usize count_str = 0;
//     for (usize i = 0; i < vector_regul.size(); i++) {
//         if (vector_regul[i] == "*") {
//             if (i + 1 < vector_regul.size())
//                 count_str = check_word(str, vector_regul[i + 1], count_str);
//             if (count_str > str.size())
//                 return false;
//             i++;
//         } else if (vector_regul[i] == "+") {
//             if (count_str > str.size() - 1)
//                 return false;
//             count_str++;
//             if (i + 1 < vector_regul.size())
//                 count_str = check_word(str, vector_regul[i + 1], count_str);
//             if (count_str > str.size())
//                 return false;
//             i++;
//         } else {
//             count_str = check_word(str, vector_regul[i], count_str);
//             if (count_str > str.size() || (i == vector_regul.size() - 1 && count_str < str.size()))
//                 return false;
//         }
//     }
//     return true;
// }

int main(int, char**) {
    Error err(0, "No error");
    Config::Category conf = Config::Category::ParseFromINI("../sources/config/config_examples/default.ini", &err);
    
    Config::Category::DumpToINI(conf, "../sources/config/config_examples/New.ini", &err);

}  
