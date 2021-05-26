#include "http.h"
#include "utils.h"
#include <sstream>

namespace Http {

static Method StringToMethod(const std::string& method_str) {
    // TODO(handrow): make it faster, could use hash codes !!!
    for (unsigned method_idx = 0; method_idx < END_METHOD; ++method_idx) {
        if (METHOD_TO_STRING[method_idx] == method_str)
            return static_cast<Method>(method_idx);
    }
    return UNKNOWN_METHOD;
}

static ProtocolVersion StringToVersion(const std::string& version_str) {
    for (unsigned version_idx = 0; version_idx < END_VERSION; ++version_idx) {
        if (VERSION_TO_STRING[version_idx] == version_str)
            return static_cast<ProtocolVersion>(version_idx);
    }
    return UNKNOWN_VERSION;
}

ParseError          Request::ParseStartLine(const std::string& start_line) {
    size_t tok_begin = 0;
    size_t tok_end = 0;
    // TODO(handrow):      In the interest of robustness, servers SHOULD ignore any empty
                        // line(s) received where a Request-Line is expected. In other words, if
                        // the server is reading the protocol stream at the beginning of a
                        // message and receives a CRLF first, it should ignore the CRLF.

    /// Method parsing
    {
        tok_end = start_line.find_first_of(' ', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
        __method = StringToMethod(start_line.substr(tok_begin, tok_end - tok_begin));
        if (__method == UNKNOWN_METHOD)
            return ERR_UNKNOWN_HTTP_METHOD;
        tok_begin = ++tok_end;  // skip space and init new begin
    }

    /// Uri parsing
    {
        tok_end = start_line.find_first_of(' ', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
        __uri.__text = start_line.substr(tok_begin, tok_end - tok_begin);
        // TODO(handrow): validation of URI
        tok_begin = ++tok_end;  // skip space and init new begin
    }

    /// Version parsing
    {
        tok_end = start_line.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            tok_end = start_line.length();
        __version = StringToVersion(start_line.substr(tok_begin, tok_end - tok_begin));
        if (__version == UNKNOWN_VERSION)
            return ERR_UNKNOWN_HTTP_VERSION;
        tok_begin = ++tok_end;
    }

    return ERR_OK;
}

ParseError          Request::ParseNewHeader(const std::string& head_str) {
    //KEY: VALUE\n
    size_t          tok_begin = 0;
    size_t          tok_end = 0;
    Headers::Pair   head;

    /// Key Parsing
    tok_end = head_str.find_first_of(':', tok_begin);
    if (tok_end == std::string::npos)
        return ERR_INVALID_HTTP_HEADER;
    head.first = head_str.substr(tok_begin, tok_end - tok_begin);
    for (size_t i = 0; i < head.first.length(); ++i) {
        if (IsSeparator(head.first[i])) {
            return ERR_INVALID_HTTP_HEADER;
        }
    }

    /// Space check and skip
    if (head_str[++tok_end] != ' ')
        return ERR_INVALID_HTTP_HEADER;
    while (isspace(head_str[tok_end]))
        tok_begin = ++tok_end;

    /// Value Parsing
    tok_end = head_str.find_first_of('\n', tok_begin);
    if (tok_end == std::string::npos)
        tok_end = head_str.length();
    size_t last_of_print = FindLastPrint(head_str);
    head.second = head_str.substr(tok_begin, last_of_print - tok_begin + 1);
    __headers.SetHeader(head.first, head.second);
    return ERR_OK;
}

ParseError          Request::ParseFromString(const std::string& req_str) {
    ParseError      error;
    size_t          tok_begin = 0;
    size_t          tok_end = 0;

    /// Ignore CRLF before Request-Line
    while (req_str[tok_end] == '\n')
        tok_begin = ++tok_end;

    /// StartLine Parsing
    {
        tok_end = req_str.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
        error = ParseStartLine(req_str.substr(tok_begin, tok_end - tok_begin));
        if (error != ERR_OK)
            return error;
        tok_begin = ++tok_end;
    }

    /// Headers Parsing
    for (;;) {
        tok_end = req_str.find_first_of('\n', tok_begin);
        if (tok_end == std::string::npos)
            return ERR_INCOMPLETE_HTTP_REQUEST;
        if (tok_end == tok_begin)  // empty string
            break;
        error = ParseNewHeader(req_str.substr(tok_begin, tok_end - tok_begin));
        if (error != ERR_OK)
            return error;
        tok_begin = ++tok_end;
    }

    /// Body Parsing
    {
        tok_begin = ++tok_end;  // skip empty line
        __body = req_str.substr(tok_begin);
    }

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
