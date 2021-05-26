#include "http.h"
#include <sstream>

namespace Http {

Response::Response(int code, const std::string& body)
: __version(UNKNOWN_VERSION)
, __headers()
, __body(body)
, __code(code) {
}

static ProtocolVersion StringToVersion(const std::string& version_str) {
    for (unsigned version_idx = 0; version_idx < END_VERSION; ++version_idx) {
        if (VERSION_TO_STRING[version_idx] == version_str)
            return static_cast<ProtocolVersion>(version_idx);
    }
    return UNKNOWN_VERSION;
}

// TODO:(handrow) check content length and data in response
// DO NOT COPY STATIC VERSION_TO_STRING FUNC
ParseError          Response::ParseStartLine(const std::string& start_line) {
    size_t tok_begin = 0;
    size_t tok_end = 0;
    std::stringstream ss;

    /// Version Parsing
    tok_end = start_line.find_first_of(' ', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INCOMPLETE_HTTP_RESPONSE;
    __version = StringToVersion(start_line.substr(tok_begin, tok_end - tok_begin));
    if (__version == UNKNOWN_VERSION)
        return ERR_UNKNOWN_HTTP_VERSION;
    tok_begin = ++tok_end;

    /// Status Code parser
    tok_end  = start_line.find_first_of(' ', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INCOMPLETE_HTTP_RESPONSE;
    ss << start_line.substr(tok_begin, tok_end - tok_begin);
    ss >> __code;
    tok_begin = ++tok_end;

    /// Code Text Parsing
    __code_phrase = GetCodeMessage(__code);

    return ERR_OK;
}

ParseError          Response::ParseNewHeader(const std::string& head_str) {
    size_t          tok_begin = 0;
    size_t          tok_end = 0;
    Headers::Pair   head;

    /// Key Parsing
    tok_end = head_str.find_first_of(':', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INVALID_HTTP_HEADER;
    head.first = head_str.substr(tok_begin, tok_end - tok_begin);

    /// Space check and skip
    if (head_str[++tok_end] != ' ')
        return ERR_INVALID_HTTP_HEADER;
    tok_begin = ++tok_end;

    /// Value Parsing
    tok_end = head_str.find_first_of('\n', tok_begin);
    if (tok_end == std::string::npos)
        tok_end = head_str.length();
    head.second = head_str.substr(tok_begin, tok_end - tok_begin);

    __headers.SetHeader(head.first, head.second);

    return ERR_OK;
}

ParseError          Response::ParseFromString(const std::string& str_response) {
    ParseError  error;
    size_t      tok_begin = 0;
    size_t      tok_end = 0;

    /// StartLine Parsing
    {
        tok_end = str_response.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_RESPONSE;
        error = ParseStartLine(str_response.substr(tok_begin, tok_end - tok_begin));
        if (error != ERR_OK)
            return error;
        tok_begin = ++tok_end;
    }

    /// Headers Parsing
    for (;;) {
        tok_end = str_response.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_RESPONSE;
        if (tok_end == tok_begin)  // empty string
            break;
        error = ParseNewHeader(str_response.substr(tok_begin, tok_end - tok_begin));
        if (error != ERR_OK)
            return error;
        tok_begin = ++tok_end;
    }

    /// Body Parsing
    {
        tok_begin = ++tok_end;  // skip empty line
        __body = str_response.substr(tok_begin);
    }

    return ERR_OK;
}

std::string         Response::ParseToString() const {
    std::string str = VERSION_TO_STRING[GetVersion()] + std::string(" ")
                    + std::to_string(GetCode()) + std::string(" ")
                    + __code_phrase + std::string("\n")
                    + __headers.ToString() + std::string("\n")
                    + __body;
    return str;
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

void                Response::SetCodeMessage(const std::string& phrase) {
    __code_phrase = phrase;
}

std::string         Response::GetCodeMessage(int code) {
    static CodeToMessageMap* __code_messages = NULL;
    if (__code_messages == NULL) {
        // need to init that one
        __code_messages = new CodeToMessageMap;
        // 1xx: Informational
        (*__code_messages)[100] = "Continue";
        (*__code_messages)[101] = "Switching Protocols";
        // 2xx: Success
        (*__code_messages)[200] = "OK";
        (*__code_messages)[201] = "Created";
        (*__code_messages)[202] = "Accepted";
        (*__code_messages)[203] = "Non-Authoritative Information";
        (*__code_messages)[204] = "No Content";
        (*__code_messages)[205] = "Reset Content";
        (*__code_messages)[206] = "Partial Content";
        // 3xx: Redirection
        (*__code_messages)[300] = "Multiple Choices";
        (*__code_messages)[301] = "Moved Permanently";
        (*__code_messages)[302] = "Found";
        (*__code_messages)[303] = "See Other";
        (*__code_messages)[304] = "Not Modified";
        (*__code_messages)[305] = "Use Proxy";
        (*__code_messages)[307] = "Temporary Redirect";
        // 4xx: Client error
        (*__code_messages)[400] = "Bad Request";
        (*__code_messages)[401] = "Unauthorized";
        (*__code_messages)[402] = "Payment Required";
        (*__code_messages)[403] = "Forbidden";
        (*__code_messages)[404] = "Not Found";
        (*__code_messages)[405] = "Method Not Allowed";
        (*__code_messages)[406] = "Not Acceptable";
        (*__code_messages)[407] = "Proxy Authentication Required";
        (*__code_messages)[408] = "Request Time-out";
        (*__code_messages)[409] = "Conflict";
        (*__code_messages)[410] = "Gone";
        (*__code_messages)[411] = "Length Required";
        (*__code_messages)[412] = "Precondition Failed";
        (*__code_messages)[413] = "Request Entity Too Large";
        (*__code_messages)[414] = "Request-URI Too Large";
        (*__code_messages)[415] = "Unsupported Media Type";
        (*__code_messages)[416] = "Requested range not satisfiable";
        (*__code_messages)[417] = "Expectation Failed";
        // 5xx: Server error
        (*__code_messages)[500] = "Internal Server Error";
        (*__code_messages)[501] = "Not Implemented";
        (*__code_messages)[502] = "Bad Gateway";
        (*__code_messages)[503] = "Service Unavailable";
        (*__code_messages)[504] = "Gateway Time-out";
        (*__code_messages)[505] = "HTTP Version not supported";
    }
    CodeToMessageMap::iterator item_it = __code_messages->find(code);
    if (item_it == __code_messages->end())
        return NULL;
    return item_it->second;
}

}  // namespace Http
