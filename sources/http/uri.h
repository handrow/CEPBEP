#ifndef HTTP_URI_H_
#define HTTP_URI_H_

#include "common/types.h"
#include "common/error.h"

#include <string>
#include <map>

namespace Http {

enum UriErrorCode {
    URI_NO_ERR = 30000,
    URI_BAD_SCHEME,
    URI_BAD_SYNTAX
};

struct URI {

    typedef std::map<std::string, std::string>  QueryMap;
    typedef std::pair<std::string, std::string> QueryPair;

    struct Authority {
        std::string __username;
        std::string __password;
        std::string __hostname;
        std::string __port;
    };

    enum Scheme {
        URI_SCHEME_UNKNOWN,
        URI_SCHEME_HTTP,
        URI_SCHEME_URL,
    };

    Scheme      __scheme;
    Authority   __auth;
    std::string __path;
    QueryMap    __query_params;
    std::string __fragment;

    static std::string  PercentEncode(const std::string& str);
    static std::string  PercentDecode(const std::string& str);

    static std::string  EncodeQuery(const QueryMap& params);
    static QueryMap     DecodeQuery(const std::string& query, Error* err);

    static std::string  EncodeScheme(Scheme scheme);
    static Scheme       DecodeScheme(const std::string& str, Error* err);

    static std::string  EncodeAuthority(const Authority& auth);
    static Authority    DecodeAuthority(const std::string& str, Error* err);

    static std::string  EncodeUri(const URI& uri);
    static URI          DecodeUri(const std::string& uri_str, Error* err);

};

}  // namespace Http

#endif  // HTTP_URL_H_
