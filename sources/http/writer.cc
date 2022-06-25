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
    Headers_ = Headers();
    Body_ = std::string();
}

Headers&    ResponseWriter::Header() {
    return Headers_;
}

const Headers&  ResponseWriter::Header() const {
    return Headers_;
}

bool  ResponseWriter::HasBody() const {
    return !Body_.empty();
}

void            ResponseWriter::Write(const std::string& body_appendix) {
    Body_ += body_appendix;
}

std::string     ResponseWriter::SendToString(int code, ProtocolVersion ver) {
    Response res;

    Headers_.Set("Content-Length", Convert<std::string>(Body_.size()));
    Headers_.Set("Date", Headers::CurrentDate());

    res.Headers = Headers_;
    res.CodeMessage = SearchCodeMap(code);
    res.Code = code;
    res.Version = ver;
    res.Body = Body_;

    Reset();

    return res.ToString();
}


}  // namespace Http
