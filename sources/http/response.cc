#include "http.h"

namespace Http {

Response::Response(int code, const std::string& body)
: __version(UNKNOWN_VERSION)
, __headers()
, __body(body)
, __code(code) {
}

Headers&            Response::GetHeadersRef() {
    return __headers;
}

const Headers&      Response::GetHeadersRef() const {
    return __headers;
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

}  // namespace Http
