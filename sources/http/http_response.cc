#include "http.h"

namespace ft {

Response::Response(int code, const std::string& body)
: __version(UNKNOWN_VERSION)
, __headers()
, __body(body)
, __code(code) {
}

std::string         Response::ParseToString() const {
    std::string str = "";
    str.append(VERSION_TO_STRING[GetVersion()]);
    str.append(std::to_string(__code));
    str.append("\n");
    HeaderToString(str);
    str.append(__body);
    return str;
}

void                Response::HeaderToString(std::string& str) const {
    for (Headers::const_iterator it = __headers.begin(); it != __headers.end(); it++) {
        str.append(it->first);
        str.append(": ");
        str.append(it->second);
        str.append("\n");
    }
}

void                Response::SetHeader(const std::string& h_name, const std::string& h_val) {
    __headers[h_name] = h_val;
}

std::string         Response::GetHeader(const std::string& h_name) const {
    Headers::const_iterator it = __headers.find(h_name);
    if (it != __headers.end())
        return it->first;
    return "";
}

HeaderDelStatus     Response::DeleteHeader(const std::string& h_name) {
    return __headers.erase(h_name) == 0 ? HEADER_DEL_FAIL : HEADER_DEL_OK;
}

void                Response::SetVersion(ProtocolVersion version) {
    __version = version;
}

ProtocolVersion     Response::GetVersion() const {
    return __version;
}

void                Response::SetBody(const std::string& body) {
    __body = body;
}

std::string         Response::GetBody() const {
    return __body;
}

void                Response::SetCode(int code) {
    __code = code;
}

int                 Response::GetCode() const {
    return __code;
}

}  // namespace ft
