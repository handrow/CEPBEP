#include "http/writer.h"

namespace Http {
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


void        ResponseWriter::Reset() {
    __hdrs = Headers();
    __body = std::string();
}

Headers&    ResponseWriter::Header() {
    return __hdrs;
}

const Headers&  ResponseWriter::Header() const {
    return __hdrs;
}

bool  ResponseWriter::HasBody() const {
    return !__body.empty();
}

void            ResponseWriter::Write(const std::string& body_appendix) {
    __body += body_appendix;
}

std::string     ResponseWriter::SendToString(int code, ProtocolVersion ver) {
    Response res;

    __hdrs.Set("Content-Length", Convert<std::string>(__body.size()));
    __hdrs.Set("Date", Headers::CurrentDate());

    res.headers = __hdrs;
    res.code_message = SearchCodeMap(code);
    res.code = code;
    res.version = ver;
    res.body = __body;

    Reset();

    return res.ToString();
}


}  // namespace Http
