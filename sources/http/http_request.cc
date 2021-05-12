#include "http.h"
#include <iostream>

namespace ft {

Request::Request(Method method)
: __version(UNKNOWN_VERSION)
, __method(method)
, __headers()
, __params()
, __body()
, __url() {
}

ParseError          Request::ParseFromString(const std::string& str_request) {
    States state = STATE_START;
    std::string method = "";
    std::string version = "";
    for (std::string::const_iterator it = str_request.begin(); it != str_request.end(); it++) {
        switch(state) {
        case STATE_START:
            while (!isspace(*it)) 
            {
                method.append(1, *it);
                it++;
            }
            if (method == "GET") {
                SetMethod(GET);
                state = STATE_URL;
            }
            else
                std::cout << "FAIL METHOD" << std::endl;
        case STATE_URL:
            while (isspace(*it))
                it++;
            if (*it == '/') {
                while (!isspace(*it) && it != str_request.end()) {
                    __url.append(1, *it);
                    it++;
                }
                state = STATE_VERSION;
            }
            else
                return ERR_FAIL;
        case STATE_VERSION:
            while (isspace(*it) && it != str_request.end())
                it++;
            while (!isspace(*it) && it != str_request.end()) {
                version.append(1, *it);
                it++;
            }
            if (version == "HTTP/1.1") {
                SetVersion(HTTP_1_1);
                break;
            }
            else
                return ERR_FAIL;  
        }
    }
    return ERR_OK;
}

std::string         Request::ParseToString() const {
    std::string str = "";
    str.append(METHOD_TO_STRING[GetMethod()]);
    str.append(GetUrl());
    QueryParamToString(str);
    str.append(VERSION_TO_STRING[GetVersion()]);
    str.append("\n");
    HeaderToString(str);
    str.append(__body);
    return str;
}

void                Request::HeaderToString(std::string& str) const {
    for (Headers::const_iterator it = __headers.begin(); it != __headers.end(); it++) {
    str.append(it->first);
    str.append(": ");
    str.append(it->second);
    str.append("\n");
    }
}

void                Request::QueryParamToString(std::string& str) const {
    for (Query::const_iterator it = __params.begin(); it != __params.end(); it++) {
        str.append("?");
        str.append(it->first);
        str.append("=");
        str.append(it->second);
    }
    str.append(" ");
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
