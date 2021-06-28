#include "common/string_utils.h"
#include "http/http.h"

namespace Http {

const char TO_STRING_OPTIONS::CRLF_SYM[] = "\r\n";


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

std::string     Request::ToString() const {
    return   MethodToString(method) + " "
           + uri.ToString() + " "
           + ProtocolVersionToString(version) + " "
           + headers.ToString() + TO_STRING_OPTIONS::CRLF_SYM
           + body;
}

std::string     Response::ToString() const {
    return   ProtocolVersionToString(version) + " "
           + Convert<std::string>(code) + " "
           + code_message + TO_STRING_OPTIONS::CRLF_SYM
           + headers.ToString() + TO_STRING_OPTIONS::CRLF_SYM
           + body;
}

}  // namespace Http
