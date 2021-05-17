
#ifndef HTTP_HTTP_H_
# define HTTP_HTTP_H_

#include <map>
#include <string>

namespace Http {

enum ParseError {
    ERR_OK,
    ERR_FAIL,
    ERR_UNKNOWN,
    ERR_UNKNOWN_HTTP_METHOD,
    ERR_UNKNOWN_HTTP_VERSION,
    ERR_INCOMPLETE_HTTP_REQUEST,
    ERR_INCOMPLETE_HTTP_RESPONSE,
    ERR_INVALID_HTTP_HEADER
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
    OPTIONS,
    // for loops
    END_METHOD
};

enum ProtocolVersion {
    UNKNOWN_VERSION,
    HTTP_1_0,
    HTTP_1_1,
    END_VERSION
};

const char* const METHOD_TO_STRING[] = {
    "UNKNOWN_METHOD",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "PATCH",
    "DELETE",
    "CONNECT",
    "TRACE",
    "OPTIONS"
};

const char* const VERSION_TO_STRING[] = {
    "UNKNOWN VERSION",
    "HTTP/1.0",
    "HTTP/1.1"
};


struct URI {
    std::string __text;
};


class Headers {
 public:
    typedef std::map<std::string, std::string>  Map;
    typedef std::pair<std::string, std::string> Pair;

    enum DelStatus {
        HEADER_DEL_FAIL,
        HEADER_DEL_OK
    };

 protected:
    Map __map;

 protected:
    static std::string HeaderPairToString(const Pair& head);

 public:
    std::string     ToString() const;

    std::string     GetHeader(const std::string& key) const;
    void            SetHeader(const std::string& key, const std::string& value);
    DelStatus       DeleteHeader(const std::string& key);
};


class Request {
 protected:
    ProtocolVersion __version;
    Method          __method;
    Headers         __headers;
    std::string     __body;
    URI             __uri;

 protected:
    ParseError          ParseStartLine(const std::string& req_str);
    ParseError          ParseNewHeader(const std::string& head_str);

 public:
    explicit
    Request(Method method = UNKNOWN_METHOD);

    // explicit
    // Request(const std::string& str_request);

    ParseError          ParseFromString(const std::string& str_request);
    std::string         ParseToString() const;

    Headers&            GetHeadersRef();
    const Headers&      GetHeadersRef() const;

    void                SetMethod(Method method);
    Method              GetMethod() const;

    void                SetVersion(ProtocolVersion version);
    ProtocolVersion     GetVersion() const;

    void                SetUri(const std::string& uri);
    std::string         GetUri() const;

    void                SetBody(const std::string& body);
    std::string         GetBody() const;
    void                AppendToBody(const std::string& appendix);
};


class Response {
 protected:
    ProtocolVersion __version;
    Headers         __headers;
    std::string     __body;
    int             __code;

 protected:
    ParseError          ParseStartLine(const std::string& start_line);
    ParseError          ParseNewHeader(const std::string& head_str);
 public:
    // explicit
    // Response(const std::string& str_response);
    Response(int code, const std::string& body);

    ParseError          ParseFromString(const std::string& str_response);
    std::string         ParseToString() const;

    Headers&            GetHeadersRef();
    const Headers&      GetHeadersRef() const;

    void                SetVersion(ProtocolVersion version);
    ProtocolVersion     GetVersion() const;

    void                SetBody(const std::string& body);
    std::string         GetBody() const;

    void                SetCode(int code);
    int                 GetCode() const;
};

}  // namespace Http

#endif  // HTTP_HTTP_H_
