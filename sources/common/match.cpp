#include "common/string_utils.h"

namespace {

inline bool IsntEnd(const std::string& str, USize i) {
    return i < str.size();
}

USize TryBlock(const std::string& reg, const std::string& str, USize ri, USize si) {
    std::string block = reg.substr(ri, reg.find("]") - ri);

    Tokenizator tkz(block);
    std::string token;
    bool run = true;

    while ( token = tkz.Next("[|", "|]", &run), run ) {
        if (token == str.substr(si, token.size()))
            return si + token.size();
    }

    return std::string::npos;
}

}  // namespace

bool Match(const std::string& reg, const std::string& str, USize ri, USize si) {
    if (IsntEnd(str, si) && (IsntEnd(reg, ri) && reg[ri] == '*')) {
        return Match(reg, str, ri, si + 1) || Match(reg, str, ri + 1, si);
    } else if (IsntEnd(str, si) && IsntEnd(reg, ri) && str[si] == reg[ri]) {
        return Match(reg, str, ri + 1, si + 1);
    } else if (reg[ri] == '[') {
        si = TryBlock(reg, str, ri, si);
        if (si != std::string::npos)
            return Match(reg, str, reg.find("]") + 1, si);
        return false;
    } else if (!IsntEnd(str, si) && !IsntEnd(reg, ri)) {
        return true;
    } else if (!IsntEnd(str, si) && reg[ri] == '*') {
        return Match(reg, str, ri + 1, si);
    }
    return false;
}
