#include <ctime>

#include "common/string_utils.h"
#include "http/http.h"

namespace Http {

std::string     Headers::CurrentDate() {
    time_t  t = std::time(NULL);
    std::tm tm;

    gmtime_r(&t, &tm);
    std::string buff;

    buff.resize(255);

    size_t len = strftime(const_cast<char *>(buff.data()), 255,
                          "%a, %d %b %Y %H:%M:%S GMT", &tm);

    buff.resize(len);
    return buff;
}

usize           Headers::GetContentLength(const Headers& hdrs) {
    usize res = 0;
    HeaderMap::const_iterator it = hdrs.__map.find("Content-Length");

    if (it != hdrs.__map.end()) {
        res = Convert<usize>(it->second);
    }
    return res;
}

bool     Headers::IsChunkedEncoding(const Headers& hdrs) {
    HeaderMap::const_iterator it = hdrs.__map.find("Transfer-Encoding");
    if (it != hdrs.__map.end()) {
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

bool           Headers::IsContentLengthed(const Headers& hdrs) {
    HeaderMap::const_iterator it = hdrs.__map.find("Content-Length");
    return it != hdrs.__map.end();
}

std::string     Headers::ToString() const {
    std::string str;

    for (Headers::HeaderMap::const_iterator it = __map.begin();
                                            it != __map.end(); ++it)
            str += it->first + ": " + it->second + TO_STRING_OPTIONS::CRLF_SYM;

    return str;
}

void         Headers::SetMap(const HeaderMap& hmap) {
    __map = hmap;
}

Headers::HeaderMap    Headers::GetMap() const {
    return __map;
}

void         Headers::Set(const std::string& hname, const std::string& hval) {
    __map[hname] = hval;
}

void         Headers::Add(const std::string& hname, const std::string& hval
                                                        , const std::string& delimiter) {
    HeaderMap::iterator it = __map.find(hname);

    if (it == __map.end()) {
        Set(hname, hval);
    } else {
        it->second += delimiter + hval;
    }
}

void  Headers::Rm(const std::string& hname) {
    __map.erase(hname);
}

bool         Headers::Has(const std::string& hname) const {
    return __map.count(hname) > 0;
}

std::string  Headers::Get(const std::string& hname) const {
    HeaderMap::const_iterator it = __map.find(hname);

    return (it != __map.end()) ? it->second
                               : "";
}

std::string  Headers::Get(const std::string& hname, bool* isset) const {
    HeaderMap::const_iterator it = __map.find(hname);

    if (it == __map.end()) {
        return safe_pointer_assign(isset, false), std::string();
    } else {
        return safe_pointer_assign(isset, true), it->second;
    }
}


}  // namespace Http
