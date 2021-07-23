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

std::string Tokenizator::NextS(const std::string& delimiter, bool* run) {
    while (__tok_end < __str.length() &&
           __str.compare(__tok_end, delimiter.length(), delimiter.c_str()) == 0) {

        __tok_end += delimiter.length();
    }
    __tok_begin = __tok_end;

    if (__tok_begin >= __str.length())
        return *run = false, "";

    __tok_end = __str.find(delimiter, __tok_begin);

    if (__tok_end == std::string::npos) {
        __tok_end = __str.length();
    }

    return __str.substr(__tok_begin, __tok_end - __tok_begin);
}

std::string  Tokenizator::NextLine(bool* run) {
    __tok_begin = __tok_end;
    if (__tok_begin >= __str.length())
        return *run = false, "";

    usize crlf_pos = __str.find("\r\n", __tok_begin);
    usize lf_pos = __str.find("\n", __tok_begin);
    std::string result;

    if (crlf_pos < lf_pos) {
        result = __str.substr(__tok_begin, crlf_pos - __tok_begin);
        __tok_end = crlf_pos + 2;

    } else if (lf_pos < crlf_pos) {
        result = __str.substr(__tok_begin, lf_pos - __tok_begin);
        __tok_end = lf_pos + 1;

    } else {
        result = __str.substr(__tok_begin);
        __tok_end = __str.length();
    }

    return result;
}

usize  Tokenizator::GetPos() const {
    return __tok_end;
}
