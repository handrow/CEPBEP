#include <sstream>
#include "http/http.h"

namespace Http {

usize       Headers::GetContentLength(const Headers& hdrs) {
    usize res = 0;
    HeaderMap::const_iterator it = hdrs.__map.find("Content-Length");

    if (it != hdrs.__map.end()) {
        std::stringstream ss;
        ss << it->second;
        ss >> res;
    }
    return res;
}

Method      MethodFromString(const std::string& method_str) {
    if (method_str == "GET")    return METHOD_GET;
    if (method_str == "POST")   return METHOD_POST;
    if (method_str == "DELETE") return METHOD_DELETE;
                                return METHOD_UNKNOWN;
}

ProtocolVersion ProtocolVersionFromString(const std::string& version_str) {
    if (version_str == "HTTP/1.1")  return HTTP_1_1;
    if (version_str == "HTTP/1.0")  return HTTP_1_0;
                                    return HTTP_NO_VERSION;
}

}  // namespace Http
