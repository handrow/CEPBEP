#ifndef HTTP_URI_H_
#define HTTP_URI_H_

#include <string>
#include <map>

#include "common/types.h"
#include "common/error.h"


namespace Http {

enum UriErrorCode {
    URI_NO_ERR = 30000,
    URI_BAD_SCHEME,
    URI_BAD_SYNTAX,
    URI_BAD_HOST,
    URI_BAD_PATH_SYNTAX,
    URI_BAD_QUERY_SYNTAX,
    URI_BAD_USERINFO
};

namespace __pct_encode_inner {
inline std::string  ByteToHexStr(UInt8 byte) {
    static const char* hex_alphabet = "0123456789ABCDEF";
    return std::string(1, hex_alphabet[byte / 16])
            + std::string(1, hex_alphabet[byte % 16]);
}
}  // namespace __pct_encode_inner

template <typename SafePredicate>  // bool ()(char c)
std::string PercentEncode(const std::string& decoded_str, SafePredicate is_safe_sym) {
    using namespace __pct_encode_inner;

    std::string encoded_str;

    for (USize sym_idx = 0; sym_idx < decoded_str.length(); ++sym_idx) {
        if (!is_safe_sym(decoded_str[sym_idx]))
            encoded_str += ("%" + ByteToHexStr(decoded_str[sym_idx]));
        else
            encoded_str += decoded_str[sym_idx];
    }

    return encoded_str;
}

std::string PercentDecode(const std::string& str);

bool IsUnrsvdSym(char c);
bool IsPathSafeSym(char c);

struct URI {
    std::string UserInfo;
    std::string Hostname;
    std::string Path;
    std::string QueryStr;
    std::string Fragment;

    std::string         ToString() const;

    static URI          Parse(const std::string& uri_str, Error* err);
};

struct Query {
    typedef std::map<std::string, std::string>  ParamMap;
    typedef ParamMap::value_type                ParamPair;

    ParamMap    Params;

    std::string         ToString() const;

    static Query        Parse(const std::string& queryStr, Error* err);
};

}  // namespace Http

#endif  // HTTP_URI_H_
