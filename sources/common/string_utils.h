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
    explicit Tokenizator(const std::string& str, usize offset = 0ULL);
    std::string Next(const char delims[], bool* run);
    std::string Next(const char bdelims[], const char edelims[], bool* run);
};

bool Match(const std::string& reg, const std::string& str, usize ri = 0, usize si = 0);

#endif  // COMMON_STRING_UTILS_H_
