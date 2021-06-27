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

class Tokenizator {
 private:
    std::string __str;
    usize       __tok_begin;
    usize       __tok_end;

 public:

    Tokenizator(const std::string& str, usize offset = 0ULL)
    : __str(str)
    , __tok_begin(offset)
    , __tok_end(offset) {
    }

    std::string Next(const char delims[], bool* run) {
        return Next(delims, delims, run);
    }

    std::string Next(const char bdelims[], const char edelims[], bool* run) {
        __tok_begin = __str.find_first_not_of(bdelims, __tok_end);
        if (__tok_begin == std::string::npos) {
            return *run = false, "";
        }
        __tok_end = __str.find_first_of(edelims, __tok_begin);
        if (__tok_end == std::string::npos)
            __tok_end = __str.length();

        return __str.substr(__tok_begin, __tok_end - __tok_begin);
    }
};

#endif  // COMMON_STRING_UTILS_H_
