#include "common/string_utils.h"

Tokenizator::Tokenizator(const std::string& str, usize offset)
: __str(str)
, __tok_begin(offset)
, __tok_end(offset) {
}

std::string Tokenizator::Next(const char delims[], bool* run) {
    return Next(delims, delims, run);
}

std::string Tokenizator::Next(const char bdelims[], const char edelims[], bool* run) {
    __tok_begin = __str.find_first_not_of(bdelims, __tok_end);
    if (__tok_begin == std::string::npos) {
        return *run = false, "";
    }
    __tok_end = __str.find_first_of(edelims, __tok_begin);
    if (__tok_end == std::string::npos)
        __tok_end = __str.length();

    return __str.substr(__tok_begin, __tok_end - __tok_begin);
}
