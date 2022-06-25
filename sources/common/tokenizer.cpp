#include "common/string_utils.h"

Tokenizator::Tokenizator(const std::string& str, USize offset)
: Str_(str)
, TokBegin_(offset)
, TokEnd_(offset) {
}

std::string Tokenizator::Next(const char delims[], bool* run) {
    return Next(delims, delims, run);
}

std::string Tokenizator::Next(const char bdelims[], const char edelims[], bool* run) {
    TokBegin_ = Str_.find_first_not_of(bdelims, TokEnd_);
    if (TokBegin_ == std::string::npos) {
        return *run = false, "";
    }
    TokEnd_ = Str_.find_first_of(edelims, TokBegin_);
    if (TokEnd_ == std::string::npos)
        TokEnd_ = Str_.length();

    return Str_.substr(TokBegin_, TokEnd_ - TokBegin_);
}

std::string Tokenizator::NextS(const std::string& delimiter, bool* run) {
    while (TokEnd_ < Str_.length() &&
           Str_.compare(TokEnd_, delimiter.length(), delimiter.c_str()) == 0) {

        TokEnd_ += delimiter.length();
    }
    TokBegin_ = TokEnd_;

    if (TokBegin_ >= Str_.length())
        return *run = false, "";

    TokEnd_ = Str_.find(delimiter, TokBegin_);

    if (TokEnd_ == std::string::npos) {
        TokEnd_ = Str_.length();
    }

    return Str_.substr(TokBegin_, TokEnd_ - TokBegin_);
}

std::string  Tokenizator::NextLine(bool* run) {
    TokBegin_ = TokEnd_;
    if (TokBegin_ >= Str_.length())
        return *run = false, "";

    USize crlfPos = Str_.find("\r\n", TokBegin_);
    USize lfPos = Str_.find("\n", TokBegin_);
    std::string result;

    if (crlfPos < lfPos) {
        result = Str_.substr(TokBegin_, crlfPos - TokBegin_);
        TokEnd_ = crlfPos + 2;

    } else if (lfPos < crlfPos) {
        result = Str_.substr(TokBegin_, lfPos - TokBegin_);
        TokEnd_ = lfPos + 1;

    } else {
        result = Str_.substr(TokBegin_);
        TokEnd_ = Str_.length();
    }

    return result;
}

USize  Tokenizator::GetPos() const {
    return TokEnd_;
}
