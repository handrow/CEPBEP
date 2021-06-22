#include "common/string_utils.h"
#include "http/http.h"

namespace Http {

usize       Headers::GetContentLength(const Headers& hdrs) {
    usize res = 0;
    HeaderMap::const_iterator it = hdrs.__map.find("Content-Length");

    if (it != hdrs.__map.end()) {
        res = Convert<usize>(it->second);
    }
    return res;
}

Method      MethodFromString(const std::string& method_str) {
    if (StrToUpper(method_str) == "GET")
        return METHOD_GET;
    if (StrToUpper(method_str) == "POST")
        return METHOD_POST;
    if (StrToUpper(method_str) == "DELETE")
        return METHOD_DELETE;
    else
        return METHOD_UNKNOWN;
}

ProtocolVersion ProtocolVersionFromString(const std::string& version_str) {
    if (StrToUpper(version_str) == "HTTP/1.1")
        return HTTP_1_1;
    if (StrToUpper(version_str) == "HTTP/1.0")
        return HTTP_1_0;
    else
        return HTTP_NO_VERSION;
}

}  // namespace Http
