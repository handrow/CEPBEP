#include "http.h"
#include <iostream>

namespace Http {

static Method StringToMethod(const std::string& method_str) {
    // TODO(handrow): make it faster, could use hash codes !!!
    for (unsigned method_idx = 0; method_idx < END_METHOD; ++method_idx) {
        if (METHOD_TO_STRING[method_idx] == method_str)
            return static_cast<Method>(method_idx);
    }
    return UNKNOWN_METHOD;
}

ParseError          Request::ParseFromString(const std::string& str) {
    __method = StringToMethod(str);
    if (__method == UNKNOWN_METHOD)
        return ERR_UNKNOWN_HTTP_METHOD;
    return ERR_OK;

}


std::string         Request::ParseToString() const {
    std::string str = METHOD_TO_STRING[GetMethod()] + std::string(" ")
                    + GetUri() + std::string(" ")
                    + VERSION_TO_STRING[GetVersion()] + std::string("\n")
                    + __headers.ToString() + std::string("\n")
                    + __body;
    return str;
}


Request::Request(Method method)
: __version(UNKNOWN_VERSION)
, __method(method)
, __headers()
, __body()
, __uri() {
}

Headers&            Request::GetHeadersRef() {
    return __headers;
}

const Headers&      Request::GetHeadersRef() const {
    return __headers;
}

void                Request::SetMethod(Method method) {
    __method = method;
}

Method              Request::GetMethod() const {
    return __method;
}

void                Request::SetVersion(ProtocolVersion version) {
    __version = version;
}

ProtocolVersion     Request::GetVersion() const {
    return __version;
}

void                Request::SetUri(const std::string& uri) {
    __uri.__text = uri;
}

std::string         Request::GetUri() const {
    return __uri.__text;
}

void                Request::SetBody(const std::string& body) {
    __body = body;
}

std::string         Request::GetBody() const {
    return __body;
}

void                Request::AppendToBody(const std::string& appendix) {
    __body.append(appendix);
}

}  // namespace Http
