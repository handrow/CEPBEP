#include "common/string_utils.h"
#include "http/reader.h"
#include <sstream>

namespace Http {
namespace __CommonHelpers {

USize IsWhiteSpace(const std::string& s, USize i) {
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
    USize tokBegin = 0;
    USize tokEnd = 0;

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Empty start line");
    }

    /// Parse method
    {
        tokEnd = buff.find_first_of(' ', tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *mtd = MethodFromString(GET_TOKEN(buff, tokBegin, tokEnd));
        if (*mtd == METHOD_UNKNOWN)
            return Error(HTTP_READER_BAD_METHOD, "Unsupported method");
        tokBegin = tokEnd;
    }

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse URI
    {
        Error uriError(0);
        tokEnd = buff.find_first_of(' ', tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *uri = URI::Parse(GET_TOKEN(buff, tokBegin, tokEnd), &uriError);
        if (uriError.IsError())
            return uriError;
        tokBegin = tokEnd;
    }

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse version
    {
        tokEnd = buff.find_first_of("\n", tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        if (buff[tokEnd - 1] == '\r')
            tokEnd -= 1;

        *ver = ProtocolVersionFromString(
                    Trim(GET_TOKEN(buff, tokBegin, tokEnd), ' '));
        if (*ver == HTTP_NO_VERSION)
            return Error(HTTP_READER_BAD_PROTOCOL_VERSION, "Bad protocol version");
    }

    return Error(Error::ERR_OK);  // no error
}

Error  ParseResponseLine(const std::string& buff, ProtocolVersion* ver, int* rcode, std::string* phrase) {
    USize tokBegin = 0;
    USize tokEnd = 0;

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Empty start line");
    }

    /// Parse version
    {
        tokEnd = buff.find_first_of(' ', tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *ver = ProtocolVersionFromString(GET_TOKEN(buff, tokBegin, tokEnd));
        if (*ver == HTTP_NO_VERSION)
            return Error(HTTP_READER_BAD_PROTOCOL_VERSION, "Unsupported version");
        tokBegin = tokEnd;
    }

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse code
    {
        tokEnd = buff.find_first_of(' ', tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        *rcode = Convert<int>(GET_TOKEN(buff, tokBegin, tokEnd));
        // if (*rcode == HTTP_READER_BAD_CODE)
        //     return Error(HTTP_READER_BAD_CODE, "Bad code");
        tokBegin = tokEnd;
    }

    /// Skip whitespaces
    {
        tokEnd = buff.find_first_not_of(' ', tokBegin);
        tokBegin = tokEnd;
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
    }

    /// Parse code phrase
    {
        tokEnd = buff.find_first_of("\n", tokBegin);
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_START_LINE, "Not full start line");
        if (buff[tokEnd - 1] == '\r')
            tokEnd -= 1;

        *phrase = Trim(GET_TOKEN(buff, tokBegin, tokEnd), ' ');
        // if (*phrase == HTTP_READER_BAD_CODE_PHRASE)
        //     return Error(HTTP_READER_BAD_CODE_PHRASE, "Bad protocol version");
    }

    return Error(Error::ERR_OK);  // no error
}

// Ignore: (CRLF)
// Valid: (HDR_KEY): *(WS) (HDR_VAL) *(WS) (CRLF)
Error  ParseHeaderPair(const std::string& pairStr, Headers* hdrs) {
    USize delimiter = 0;
    USize end = pairStr.size() - 1;

    std::string key;
    std::string val;

    if (pairStr[end] == '\n' && end == 0)  // Empty line ( pairStr == "\n" )
        return Error(0);
    if (pairStr[end - 1] == '\r')
        end -= 1;
    if (end == 0)  // Empty line ( pairStr == "\r\n" )
        return Error(0);

    /// find of pair delimiter
    delimiter = pairStr.find_first_of(':');  // IT MUST BE FOUNDED, our pair isn't empty line
    if (NOT_FOUND(delimiter))
        return Error(HTTP_READER_BAD_HEADER_KEY, "Bad header key");

    key = GET_TOKEN(pairStr, 0, delimiter);
    if (key.empty() || Back(key) == ' ')
        return Error(HTTP_READER_BAD_HEADER_KEY, "Bad header key");

    /// Key validation
    for (USize i = 0; i < key.size(); ++i) {
        if (!__CommonHelpers::IsValidHdrKey(key[i]))
            return Error(HTTP_READER_BAD_HEADER_KEY, "Bad header key");
    }

    /// Moving delimiter to first not whitespace symbol
    delimiter = pairStr.find_first_not_of(' ', delimiter + 1);
    /// Moving end to the first space in the last spaces :)
    end = pairStr.find_last_not_of(' ', end - 1) + 1;

    val = GET_TOKEN(pairStr, delimiter, end);
    /// Value validation
    for (USize i = 0; i < val.size(); ++i) {
        if (!__CommonHelpers::IsValidHdrValue(val[i]))
            return Error(HTTP_READER_BAD_HEADER_VALUE, "Bad header value");
    }

    hdrs->Add(key, val);

    return Error(0);
}

Error  ParseHeaders(const std::string& buff, Headers* hdrs) {
    USize tokBegin = 0;
    USize tokEnd = 0;
    Error err(0);

    for (;;) {
        tokEnd = buff.find("\n", tokBegin);
        if (NOT_FOUND(tokEnd)) {
            break;
        }
        tokEnd += 1;

        err = ParseHeaderPair(GET_TOKEN(buff, tokBegin, tokEnd), hdrs);
        if (err.IsError())
            return err;

        tokBegin = tokEnd;
    }

    return Error(0);
}

namespace {

USize HexNumFromString(const std::string& hexStr) {
    USize num;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> num;

    return num;
}

}  // namespace

Error  ParseChunkSize(const std::string& buff, USize* chunkSize) {
    std::string str = buff;

    if (Back(str) == '\n') {
        str.resize(str.size() - 1);
    }
    if (Back(str) == '\r') {
        str.resize(str.size() - 1);
    }

    *chunkSize = HexNumFromString(str);
    return Error(Error::ERR_OK);  // no error
}

Error  ParseCgiStatus(const Headers& hdrs, ProtocolVersion* ver, int* rcode, std::string* phrase) {
    bool isset = true;
    std::string statusValue = hdrs.Get("Status", &isset);
    if (isset) {
        USize tokBegin = 0;
        USize tokEnd = 0;

        tokEnd = statusValue.find_first_of(' ');
        if (NOT_FOUND(tokEnd))
            return Error(HTTP_READER_BAD_CODE, "Bad cgi code");
        *rcode = Convert<int>(GET_TOKEN(statusValue, tokBegin, tokEnd));
        tokBegin = tokEnd;

        tokBegin = tokEnd;
        tokEnd = statusValue.length();
        *phrase = Trim(GET_TOKEN(statusValue, tokBegin, tokEnd), ' ');
    } else {
        *rcode = 200;
        *phrase = "OK";
    }
    *ver = HTTP_1_1;

    return Error(Error::ERR_OK);
}

}  // namespace __CommonParsers

}  // namespace Http
