#ifndef HTTP_URI_H_
#define HTTP_URI_H_

#include <string>
#include <map>

namespace Http {

struct URI {

    typedef std::map<std::string, std::string> QueryMap;

    struct Authority {
        std::string __username;
        std::string __password;
        std::string __domain;
        std::string __port;
    };

    enum Scheme {
        URI_SCHEME_UNKNOWN,
        URI_SCHEME_HTTP,
    };

    Scheme      __scheme;
    Authority   __auth;
    std::string __path;
    QueryMap    __query_params;
    std::string __fragment;

    static std::string PercentEncode(const std::string& str);
    static std::string PercentDecode(const std::string& str);

    static std::string EncodeQuery(const QueryMap& params);
    static QueryMap DecodeQuery(const std::string& query);

    static std::string EncodeScheme(Scheme schema);
    static Scheme DecodeScheme(const std::string& str);

    static std::string EncodeAuthority(const Authority& auth);
    static Authority DecodeAuthority(const std::string& str);

    static std::string EncodeUri(const URI& uri);
    static URI DecodeUri(const std::string& uri_str);
};


}  // namespace Http

#endif  // HTTP_URL_H_
