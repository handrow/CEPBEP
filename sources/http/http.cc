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

std::string     Request::ToString() const{
    std::string str;
    usize content_len = Headers::GetContentLength(headers);

    str += MethodToString(method) + " "
        + uri.ToString() + " "
        + ProtocolVersionToString(version) + "\n";

    if (headers.__map.size() > 0) {
        for (Headers::HeaderMap::const_iterator it = headers.__map.begin();
                                                it != headers.__map.end(); ++it)
            str += it->first + ": " + it->second + "\n";
    }
    str += "\n";

    if (method == METHOD_POST && content_len > 0)
        str += body.substr(0, content_len);

    return str;
}

namespace {

typedef std::map<int, std::string>  CodeMap;

std::string SearchCodeMap(int code) {
    static CodeMap* map;

    if (map == NULL) {
        map = new CodeMap;
        // Information
        (*map)[100] = "Continue";
        (*map)[101] = "Switching Protocol";
        (*map)[102] = "Processing";
        (*map)[103] = "Early Hints";

        // Success
        (*map)[200] = "OK";
        (*map)[201] = "Created";
        (*map)[202] = "Accepted";
        (*map)[203] = "Non-Authoritative Information";
        (*map)[204] = "No Content";
        (*map)[205] = "Reset Content";
        (*map)[206] = "Partial Content";

        // Redirection
        (*map)[300] = "Multiple Choices";
        (*map)[301] = "Moved Permanently";
        (*map)[302] = "Found";
        (*map)[303] = "See Other";
        (*map)[304] = "Not Modified";
        (*map)[305] = "Use Proxy";
        (*map)[306] = "Switch Proxy";
        (*map)[307] = "Temporary Redirect";
        (*map)[308] = "Permanent Redirect";

        // Client Error
        (*map)[400] = "Bad Request";
        (*map)[401] = "Unauthorized";
        (*map)[402] = "Payment Required";
        (*map)[403] = "Forbidden";
        (*map)[404] = "Not Found";
        (*map)[405] = "Method Not Allowed";
        (*map)[406] = "Not Acceptable";
        (*map)[407] = "Proxy Authentication Required";
        (*map)[408] = "Request Timeout";
        (*map)[409] = "Conflict";
        (*map)[410] = "Gone";
        (*map)[411] = "Length Required";
        (*map)[412] = "Precondition Failed";
        (*map)[413] = "Request Entity Too Large";
        (*map)[414] = "Request-URI Too Long";
        (*map)[415] = "Unsupported Media Type";
        (*map)[416] = "Requested Range Not Satisfiable";
        (*map)[417] = "Expectation Failed";

        // Server Error
        (*map)[500] = "Internal Server Error";
        (*map)[501] = "Not Implemented";
        (*map)[502] = "Bad Gateway";
        (*map)[503] = "Service Unavailable";
        (*map)[504] = "Gateway Timeout";
        (*map)[505] = "HTTP Version Not Supported";
    }

    CodeMap::const_iterator it = map->find(code);

    if (it != map->end())
        return it->second;
    return "Unknown phrase";
}

}  // namespace

std::string     Response::ToString() const {

    std::string str;
    std::string phrase = SearchCodeMap(code);
    usize       content_len = Headers::GetContentLength(headers);
                content_len = (content_len == 0) ? body.length()
                                                 : content_len;

    str += ProtocolVersionToString(version) + " "
        +  std::to_string(code) + " "
        +  phrase + "\n";

    if (headers.__map.size() > 0) {
        for (Headers::HeaderMap::const_iterator it = headers.__map.begin();
                                                it != headers.__map.end(); ++it)
            str += it->first + ": " + it->second + "\n";
    }
    str += "\n";
    str += body.substr(0, content_len);

    return str;
}

}  // namespace Http
