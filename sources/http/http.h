#ifndef HTTP_HTTP_H_
#define HTTP_HTTP_H_

#include <map>
#include <string>

#include "http/uri.h"
#include "common/types.h"
#include "common/string_utils.h"

namespace Http {

/// Static class for constants
struct TO_STRING_OPTIONS {
    static const char  CRLF_SYM[];
};

enum ProtocolVersion {
    HTTP_NO_VERSION = 0,
    HTTP_1_0,
    HTTP_1_1,
};

ProtocolVersion ProtocolVersionFromString(const std::string& versionStr);
std::string     ProtocolVersionToString(const ProtocolVersion& ver);


enum Method {
    METHOD_UNKNOWN,
    METHOD_GET,
    METHOD_POST,
    METHOD_DELETE,
    METHODS_NUM
};

Method          MethodFromString(const std::string& methodStr);
std::string     MethodToString(const Method& method);

struct Headers {
 public:
    typedef std::map<std::string, std::string, CaseInsensitiveLess> HeaderMap;
    typedef HeaderMap::value_type                                   HeaderPair;

 private:
    HeaderMap       Map_;

 public:
    /// HELPER FUNCTIONS
    static USize    GetContentLength(const Headers& hdrs);
    static bool     IsContentLengthed(const Headers& hdrs);
    static bool     IsChunkedEncoding(const Headers& hdrs);
    static std::string CurrentDate();

    void            SetMap(const HeaderMap& hmap);
    HeaderMap       GetMap() const;

    bool            Has(const std::string& hname) const;
    std::string     Get(const std::string& hname) const;
    std::string     Get(const std::string& hname, bool* isset) const;
    void            Rm(const std::string& hname);
    void            Set(const std::string& hname, const std::string& hval);
    void            Add(const std::string& hname, const std::string& hval
                                                , const std::string& delimiter = ", ");

    std::string     ToString() const;
};

struct Request {
    ProtocolVersion Version;
    URI             Uri;
    Method          Method;
    Headers         Headers;
    std::string     Body;

    std::string     ToString() const;
};

struct Response {
    ProtocolVersion Version;
    int             Code;
    std::string     CodeMessage;
    Headers         Headers;
    std::string     Body;

    std::string     ToString() const;
};

}  // namespace Http

#endif  // HTTP_HTTP_H_
