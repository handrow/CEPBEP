#include "common/string_utils.h"
#include "http/http.h"

namespace Http {

usize           Headers::GetContentLength(const Headers& hdrs) {
    usize res = 0;
    HeaderMap::const_iterator it = hdrs.__map.find("Content-Length");

    if (it != hdrs.__map.end()) {
        res = Convert<usize>(it->second);
    }
    return res;
}

Method          MethodFromString(const std::string& method_str) {
    if (StrToUpper(method_str) == "GET")
        return METHOD_GET;
    if (StrToUpper(method_str) == "POST")
        return METHOD_POST;
    if (StrToUpper(method_str) == "DELETE")
        return METHOD_DELETE;
    else
        return METHOD_UNKNOWN;
}

std::string     MethodToString(const Method& method) {
    if (method == METHOD_GET)
        return "GET";
    if (method == METHOD_POST)
        return "POST";
    if (method == METHOD_DELETE)
        return "DELETE";
    else
        return "";
}

ProtocolVersion ProtocolVersionFromString(const std::string& version_str) {
    if (StrToUpper(version_str) == "HTTP/1.1")
        return HTTP_1_1;
    if (StrToUpper(version_str) == "HTTP/1.0")
        return HTTP_1_0;
    else
        return HTTP_NO_VERSION;
}

std::string     ProtocolVersionToString(const ProtocolVersion& ver) {
    if (ver == HTTP_1_1)
        return "HTTP/1.1";
    if (ver == HTTP_1_0)
        return "HTTP/1.0";
    else
        return "";
}

std::string     RequestToString(const Request& req) {
    std::string str;
    str += MethodToString(req.method) + " "
        + req.uri.ToString() + " "
        + ProtocolVersionToString(req.version) + "\n";

    if (req.headers.__map.size() != 0) {
        for (Headers::HeaderMap::const_iterator it = req.headers.__map.begin();
                                                it != req.headers.__map.end(); ++it)
            str += it->first + ": " + it->second + "\r\n";
    }
    str += "\n";

    if (Headers::GetContentLength(req.headers) > 0)
        str += req.body;

    return str;
}

std::string     ResponseToString(const Response& res) {

    // preparing:
    //  - Check phrase and make it default for some error code
    //  - Check Content-length and calculate it if it not exists

    std::string str;
    str += ProtocolVersionToString(res.version) + " "
        +  std::to_string(res.code) + " "
        +  res.code_message + "\n";

    if (res.headers.__map.size() != 0) {
        for (Headers::HeaderMap::const_iterator it = res.headers.__map.begin();
                                                it != res.headers.__map.end(); ++it)
            str += it->first + ": " + it->second + "\r\n";
    }
    str += "\n";

    if (Headers::GetContentLength(res.headers) > 0)
        str += res.body;
    // res.body.subsrt(0. content_len)

    return str;
}

}  // namespace Http
