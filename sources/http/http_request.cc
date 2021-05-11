#include "http.h"

namespace ft {

Request::Request(Method method)
: __version(UNKNOWN_VERSION)
, __method(method)
, __headers()
, __params()
, __body()
, __url() {
}

void                Request::SetHeader(const std::string& h_name, const std::string& h_val) {
    __headers[h_name] = h_val;
}

std::string         Request::GetHeader(const std::string& h_name) const {
    Headers::const_iterator it = __headers.find(h_name);
    if (it != __headers.end())
        return it->first;
    return "";
}

HeaderDelStatus     Request::DeleteHeader(const std::string& h_name) {
    return __headers.erase(h_name) == 0 ? HEADER_DEL_FAIL : HEADER_DEL_OK;
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

void                Request::SetUrl(const std::string& url) {
    __url = url;
}

std::string         Request::GetUrl() const {
    return __url;
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

void                Request::SetQueryParam(const std::string& p_name, const std::string& p_val) {
    __params[p_name] = p_val;
}

std::string         Request::GetQueryParam(const std::string& p_name) const {
    Query::const_iterator it = __params.find(p_name);
    if (it != __params.end())
        return it->first;
    return "";
}

ParamsDelStatus     Request::DeleteQueryParam(const std::string& p_name) {
    return __params.erase(p_name) == 0 ? PARAM_DEL_FAIL : PARAM_DEL_OK;
}

}  // namespace ft
