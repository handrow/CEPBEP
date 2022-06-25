#include <ctime>

#include "common/time.h"
#include "common/string_utils.h"

#include "http/http.h"

namespace Http {

std::string  Headers::CurrentDate() {
    return FormatTimeToStr("%a, %d %b %Y %H:%M:%S GMT", std::time(NULL));
}

USize  Headers::GetContentLength(const Headers& hdrs) {
    USize res = 0;
    HeaderMap::const_iterator it = hdrs.Map_.find("Content-Length");

    if (it != hdrs.Map_.end()) {
        res = Convert<USize>(it->second);
    }
    return res;
}

bool  Headers::IsChunkedEncoding(const Headers& hdrs) {
    HeaderMap::const_iterator it = hdrs.Map_.find("Transfer-Encoding");
    if (it != hdrs.Map_.end()) {
        Tokenizator tkz(it->second);
        std::string token;
        bool run = true;

        while (token = tkz.Next(", ", &run), run) {
            if (token == "chunked")
                return true;
        }
    }
    return false;
}

bool  Headers::IsContentLengthed(const Headers& hdrs) {
    HeaderMap::const_iterator it = hdrs.Map_.find("Content-Length");
    return it != hdrs.Map_.end();
}

std::string  Headers::ToString() const {
    std::string str;

    for (Headers::HeaderMap::const_iterator it = Map_.begin();
                                            it != Map_.end(); ++it)
            str += it->first + ": " + it->second + TO_STRING_OPTIONS::CRLF_SYM;

    return str;
}

void  Headers::SetMap(const HeaderMap& hmap) {
    Map_ = hmap;
}

Headers::HeaderMap  Headers::GetMap() const {
    return Map_;
}

void  Headers::Set(const std::string& headerKey, const std::string& headerValue) {
    Map_[headerKey] = headerValue;
}

void  Headers::Add(
    const std::string& headerKey,
    const std::string& headerValue,
    const std::string& delimiter
) {
    HeaderMap::iterator it = Map_.find(headerKey);

    if (it == Map_.end()) {
        Set(headerKey, headerValue);
    } else {
        it->second += delimiter + headerValue;
    }
}

void  Headers::Rm(const std::string& headerKey) {
    Map_.erase(headerKey);
}

bool  Headers::Has(const std::string& headerKey) const {
    return Map_.count(headerKey) > 0;
}

std::string  Headers::Get(const std::string& headerKey) const {
    HeaderMap::const_iterator it = Map_.find(headerKey);

    return (it != Map_.end()) ? it->second
                               : "";
}

std::string  Headers::Get(const std::string& headerKey, bool* isset) const {
    HeaderMap::const_iterator it = Map_.find(headerKey);

    if (it == Map_.end()) {
        return AssignPtrSafely(isset, false), std::string();
    } else {
        return AssignPtrSafely(isset, true), it->second;
    }
}


}  // namespace Http
