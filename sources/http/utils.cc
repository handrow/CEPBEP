#include "utils.h"

namespace Http {

bool    IsSeparator(int c) {
    return (iscntrl(c) || isspace(c) 
        || c == '(' || c == ')' || c == '<' || c == '>' || c == '@'
        || c == ',' || c == ';' || c == ':' || c == '\\' 
        || c == '/' || c == '[' || c == ']' || c == '?' || c == '"'
        || c == '=' || c == '{' || c == '}' || c == 127);
}

bool    IsPrint(int c) {
    return (c >= 33 && c <= 126);
}

size_t  FindLastPrint(const std::string& str) {
    for (size_t i = str.length() - 1; i >= 0; --i) {
        if (IsPrint(str[i]))
            return i;
    }
    return 0;
}

bool    IsUnrsvdSym(char sym) {
    return (isalnum(sym)
            || sym == '-'
            || sym == '.'
            || sym == '~'
            || sym == '_');
}

} // namespace Http