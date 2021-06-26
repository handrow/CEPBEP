#ifndef COMMON_STRING_UTILS_H_
#define COMMON_STRING_UTILS_H_

#include <string>
#include <sstream>
#include "common/types.h"

inline std::string StrToLower(const std::string& str) {
    std::string lower;

    for (usize i = 0; i < str.size(); ++i) {
        lower += tolower(str[i]);
    }

    return lower;
}

inline std::string StrToUpper(const std::string& str) {
    std::string upper;

    for (usize i = 0; i < str.size(); ++i) {
        upper += toupper(str[i]);
    }

    return upper;
}

inline std::string Trim(const std::string& str, char sym) {
    usize begin = str.find_first_not_of(sym);
    usize end = str.find_last_not_of(sym);

    if (begin == std::string::npos) {
        begin = 0;
    }
    if (end != std::string::npos) {
        end += 1;
    }

    return str.substr(begin, end - begin);
}

struct CaseInsensitiveLess {
    inline bool operator()(const std::string& s1, const std::string& s2) const {
        return StrToLower(s1) < StrToLower(s2);
    }
};

template < typename TypoA, typename TypoB >
TypoA Convert(const TypoB& str) {
    std::stringstream ss;
    TypoA val;

    ss << str;
    ss >> val;

    return val;
}

#endif  // COMMON_STRING_UTILS_H_
