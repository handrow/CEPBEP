
#ifndef HTTP_HTTP_H_
# define HTTP_HTTP_H_

#include <map>
#include <string>

namespace ft::Http {

enum ParseError {
    ERR_OK,
    ERR_UNKNOWN,
};

enum Method {
    UNKNOWN_METHOD,
    GET,
    HEAD,
    POST,
    PUT,
    PATCH,
    DELETE,
    CONNECT,
    TRACE,
    OPTIONS
};

enum ProtocolVersion {
    UNKNOWN_VERSION,
    HTTP_1_0,
    HTTP_1_1
};

typedef std::map<std::string, std::string> Headers;
typedef std::map<std::string, std::string> Query;

class Request {
 protected:
    ProtocolVersion __version;
    Method          __method;
    Headers         __headers;
    Query           __params;
    std::string     __body;
    std::string     __url;

 public:
    explicit
    Request(Method metdhod = UNKNOWN_METHOD);

    explicit
    Request(const std::string& str_request);

    ParseError          ParseFromString(const std::string& str_request);
    std::string         ParseToString() const;

    void                SetHeader(const std::string& h_name, const std::string& h_val);
    std::string         GetHeader(const std::string& h_name) const;

    void                SetMethod(Method method);
    Method              GetMethod() const;

    void                SetVersion(ProtocolVersion version);
    ProtocolVersion     GetVersion() const;

    void                SetUrl(const std::string& url);
    std::string         GetUrl() const;

    void                SetBody(const std::string& body);
    std::string         GetBody() const;
    void                AppendToBody(const std::string& appendix);

    void                SetQueryParam(const std::string& p_name, const std::string& p_val);
    std::string         GetQueryParam(const std::string& p_name) const;
};

class Response {
 protected:
    ProtocolVersion __version;
    Headers         __headers;
    std::string     __body;
    int             __code;

 public:
    explicit
    Response(const std::string& str_response);

    ParseError          ParseFromString(const std::string& str_response);
    std::string         ParseToString() const;

    void                SetHeader(const std::string& h_name, const std::string& h_val);
    std::string         GetHeader(const std::string& h_name) const;

    void                SetVersion(ProtocolVersion version);
    ProtocolVersion     GetVersion() const;

    void                SetBody(const std::string& body);
    std::string         GetBody() const;

    void                SetCode(int c);
    int                 GetCode() const;
};

}  // namespace ft::Http

#endif  // HTTP_HTTP_H_
