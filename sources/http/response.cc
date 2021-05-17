#include "http.h"
#include <sstream>

namespace Http {

Response::Response(int code, const std::string& body)
: __version(UNKNOWN_VERSION)
, __headers()
, __body(body)
, __code(code) {
}

// TODO:(handrow) add StringToVersion func to response.cc
// check content length and data in response
// make status code and status code text
// check if error code is always equal to text code
ParseError          Response::ParseStartLine(const std::string& start_line) {
    size_t tok_begin = 0;
    size_t tok_end = 0;
    std::stringstream ss;

    /// Version Parsing
    tok_end = start_line.find_first_of(' ', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INCOMPLETE_HTTP_RESPONSE;
    //__version = StringToVersion(start_line.substr(tok_begin, tok_end - tok_begin));
    if (__version == UNKNOWN_VERSION)
        return ERR_UNKNOWN_HTTP_VERSION;
    tok_begin = ++tok_end;

    /// Status Code parser
    tok_end  = start_line.find_first_of(' ', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INCOMPLETE_HTTP_RESPONSE;
    ss << start_line.substr(tok_begin, tok_end - tok_begin);
    ss >> __code; // check code
    tok_begin = ++tok_end;

    /// Code Text Parsing

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
    std::string line;
    size_t      tok_begin = 0;
    size_t      tok_end = 0;

    /// StartLine Parsing
    {
        tok_end = str_response.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
        error = ParseStartLine(str_response.substr(tok_begin, tok_end - tok_begin));
        if (error != ERR_OK)
            return error;
        tok_begin = ++tok_end;
    }

    /// Headers Parsing
    for (;;) {
        tok_end = str_response.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
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
