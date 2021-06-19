#ifndef HTTP_HTTP_H_
#define HTTP_HTTP_H_

#include <map>
#include <string>

#include "http/uri.h"
#include "common/types.h"

namespace Http {

enum ProtocolVersion {
    HTTP_NO_VERSION = 0,
    HTTP_1_0,
    HTTP_1_1,
};

ProtocolVersion ProtocolVersionFromString(const std::string& version_str);

enum Method {
    METHOD_UNKNOWN,
    METHOD_GET,
    METHOD_POST,
    METHOD_DELETE,
};

Method MethodFromString(const std::string& method_str);

struct Headers {
    typedef std::map<std::string, std::string> HeaderMap;
    typedef HeaderMap::value_type              HeaderPair;

    /// PROPERTIES
    HeaderMap       __map;

    /// HELPER FUNCTIONS
    static usize    GetContentLength(const Headers& hdrs);
};

struct Request {
    ProtocolVersion version;
    URI             uri;
    Method          method;
    Headers         headers;
    std::string     body;
};

struct Response {
    ProtocolVersion version;
    int             code;
    std::string     code_message;
    Headers         headers;
    std::string     body;
};

}  // namespace Http

#endif  // HTTP_HTTP_H_
