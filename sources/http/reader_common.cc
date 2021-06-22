#include "common/string_utils.h"
#include "http/reader.h"
#include <sstream>

namespace Http {
namespace __CommonHelpers {

usize IsWhiteSpace(const std::string& s, usize i) {
    return (s[i] == ' ') ? 1
                         : 0;
}

bool IsSeparator(char sym) {
    return sym == '(' || sym == ')' ||
           sym == '<' || sym == '>' ||
           sym == '@' || sym == ',' ||
           sym == ';' || sym == ':' ||
           sym == '\\'|| sym == '"' ||
           sym == '/' || sym == '[' ||
           sym == ']' || sym == ']' ||
           sym == '?' || sym == '=' ||
           sym == '{' || sym == '}' ||
           sym == 32  || sym == 9;
}

bool IsValidHdrKey(char sym) {

    return (sym > 31 && sym < 127) || IsSeparator(sym);
}

bool IsValidHdrValue(char sym) {
    return IsValidHdrKey(sym);
}

}  // namespace __CommonHelpers

namespace __CommonParsers {

#define GET_TOKEN(buff, tb, te) ((buff).substr((tb), (te) - (tb)))
#define NOT_FOUND(idx)          ((idx) == (std::string::npos))

Error  ParseRequestLine(const std::string& buff, Method* mtd, URI* uri, ProtocolVersion* ver) {
    usize tok_begin = 0;
    usize tok_end = 0;

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Empty start line");
    }

    /// Parse method
    {
        tok_end = buff.find_first_of(' ', tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *mtd = MethodFromString(GET_TOKEN(buff, tok_begin, tok_end));
        if (*mtd == METHOD_UNKNOWN)
            return Error(HTTP_READER_BAD_METHOD, "Unsupported method");
        tok_begin = tok_end;
    }

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse URI
    {
        Error uri_err(0);
        tok_end = buff.find_first_of(' ', tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *uri = URI::Parse(GET_TOKEN(buff, tok_begin, tok_end), &uri_err);
        if (uri_err.IsError())
            return uri_err;
        tok_begin = tok_end;
    }

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse version
    {
        tok_end = buff.find_first_of("\n", tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        if (buff[tok_end - 1] == '\r')
            tok_end -= 1;

        *ver = ProtocolVersionFromString(
                    Trim(GET_TOKEN(buff, tok_begin, tok_end), ' '));
        if (*ver == HTTP_NO_VERSION)
            return Error(HTTP_READER_BAD_PROTOCOL_VERSION, "Bad protocol version");
    }

    return Error(Error::ERR_OK);  // no error
}

Error  ParseResponseLine(const std::string& buff, ProtocolVersion* ver, int* rcode, std::string* phrase) {
    usize tok_begin = 0;
    usize tok_end = 0;

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Empty start line");
    }

    /// Parse version
    {
        tok_end = buff.find_first_of(' ', tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *ver = ProtocolVersionFromString(GET_TOKEN(buff, tok_begin, tok_end));
        if (*ver == HTTP_NO_VERSION)
            return Error(HTTP_READER_BAD_PROTOCOL_VERSION, "Unsupported version");
        tok_begin = tok_end;
    }

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse code
    {
        tok_end = buff.find_first_of(' ', tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *rcode = Convert<int>(GET_TOKEN(buff, tok_begin, tok_end));
        // if (*rcode == HTTP_READER_BAD_CODE)
        //     return Error(HTTP_READER_BAD_CODE, "Bad code");
        tok_begin = tok_end;
    }

    /// Skip whitespaces
    {
        tok_end = buff.find_first_not_of(' ', tok_begin);
        tok_begin = tok_end;
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse code phrase
    {
        tok_end = buff.find_first_of("\n", tok_begin);
        if (NOT_FOUND(tok_end))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        if (buff[tok_end - 1] == '\r')
            tok_end -= 1;

        *phrase = Trim(GET_TOKEN(buff, tok_begin, tok_end), ' ');
        // if (*phrase == HTTP_READER_BAD_CODE_PHRASE)
        //     return Error(HTTP_READER_BAD_CODE_PHRASE, "Bad protocol version");
    }

    return Error(Error::ERR_OK);  // no error
}

// Ignore: (CRLF)
// Valid: (HDR_KEY): *(WS) (HDR_VAL) *(WS) (CRLF)
Error  ParseHeaderPair(const std::string& pair_str, Headers* hdrs) {
    usize delimiter = 0;
    usize end = pair_str.size() - 1;

    std::string key;
    std::string val;

    if (pair_str[end] == '\n' && end == 0)  // Empty line ( pair_str == "\n" )
        return Error(0);
    if (pair_str[end - 1] == '\r')
        end -= 1;
    if (end == 0)  // Empty line ( pair_str == "\r\n" )
        return Error(0);

    /// find of pair delimiter
    delimiter = pair_str.find_first_of(':');  // IT MUST BE FOUNDED, our pair isn't empty line
    if (NOT_FOUND(delimiter))
        return Error(HTTP_READER_BAD_HEADER_KEY, "Bad header key");

    key = GET_TOKEN(pair_str, 0, delimiter);
    /// Key validation
    for (usize i = 0; i < key.size(); ++i) {
        if (!__CommonHelpers::IsValidHdrKey(key[i]))
            return Error(HTTP_READER_BAD_HEADER_KEY, "Bad header key");
    }

    /// Moving delimiter to first not whitespace symbol
    delimiter = pair_str.find_first_not_of(' ', delimiter + 1);
    /// Moving end to the first space in the last spaces :)
    end = pair_str.find_last_not_of(' ', end - 1) + 1;

    val = GET_TOKEN(pair_str, delimiter, end);
    /// Value validation
    for (usize i = 0; i < val.size(); ++i) {
        if (!__CommonHelpers::IsValidHdrValue(val[i]))
            return Error(HTTP_READER_BAD_HEADER_VALUE, "Bad header value");
    }

    hdrs->__map[key] = val;
    /// I am the Programmer 😎
    return Error(0);
}

Error  ParseHeaders(const std::string& buff, Headers* hdrs) {
    usize tok_begin = 0;
    usize tok_end = 0;
    Error err(0);

    for (;;) {
        tok_end = buff.find("\n", tok_begin);
        if (NOT_FOUND(tok_end)) {
            break;
        }
        tok_end += 1;

        err = ParseHeaderPair(GET_TOKEN(buff, tok_begin, tok_end), hdrs);
        if (err.IsError())
            return err;

        tok_begin = tok_end;
    }

    return Error(0);
}

}  // namespace __CommonParsers

}  // namespace Http
