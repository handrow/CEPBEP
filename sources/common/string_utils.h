#ifndef COMMON_STRING_UTILS_H_
#define COMMON_STRING_UTILS_H_

#include <string>
#include <sstream>
#include <cstring>

#include "common/platform.h"
#include "common/types.h"

inline std::string GetFileExt(const std::string& str) {
    std::string basename = str;

    USize dsep = str.find_last_of("/");

    if (dsep != std::string::npos)
        basename = basename.substr(dsep);

    USize esep = basename.find_last_of(".");

    if (esep != std::string::npos && esep != 0)
        return basename.substr(esep + 1);

    return "";
}

inline std::string StrToLower(const std::string& str) {
    std::string lower;

    for (USize i = 0; i < str.size(); ++i) {
        lower += tolower(str[i]);
    }

    return lower;
}

inline std::string StrToUpper(const std::string& str) {
    std::string upper;

    for (USize i = 0; i < str.size(); ++i) {
        upper += toupper(str[i]);
    }

    return upper;
}

inline std::string Trim(const std::string& str, char sym) {
    USize begin = str.find_first_not_of(sym);
    USize end = str.find_last_not_of(sym);

    if (begin == std::string::npos) {
        begin = str.length();
    }
    if (end != std::string::npos) {
        end += 1;
    }

    return str.substr(begin, end - begin);
}

inline char Back(const std::string& str) {
    const USize last_idx = str.length() - USize(str.length() != 0);
    return str[last_idx];
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
    std::string Str_;
    USize       TokBegin_;
    USize       TokEnd_;

 public:
    explicit Tokenizator(const std::string& str, USize offset = 0ULL);
    std::string Next(const char delims[], bool* run);
    std::string Next(const char bdelims[], const char edelims[], bool* run);

    std::string NextS(const std::string& delimiter, bool* run);
    std::string NextLine(bool* run);

    USize GetPos() const;
};

bool Match(const std::string& reg, const std::string& str, USize ri = 0, USize si = 0);

#endif  // COMMON_STRING_UTILS_H_
