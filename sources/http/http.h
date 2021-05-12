
#ifndef HTTP_HTTP_H_
# define HTTP_HTTP_H_

#include <map>
#include <string>

namespace ft {

enum ParseError {
    ERR_OK,
    ERR_FAIL,
    ERR_UNKNOWN,
};

enum States {
    STATE_START,
    // STATE_METHOD,
    STATE_URL,
    STATE_VERSION,
    // STATE_HEADER,
    // STATE_BODY,
};

enum HeaderDelStatus {
    HEADER_DEL_FAIL,
    HEADER_DEL_OK
};

enum ParamsDelStatus {
    PARAM_DEL_FAIL,
    PARAM_DEL_OK
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

class Request {
 protected:
    ProtocolVersion __version;
    Method          __method;
    Headers         __headers;
    Query           __params;
    std::string     __body;
    std::string     __url;
 
 private:
    void                HeaderToString(std::string& str) const;
    void                QueryParamToString(std::string& str) const;

 public:
    explicit
    Request(Method method = UNKNOWN_METHOD);

    // explicit
    // Request(const std::string& str_request);

    ParseError          ParseFromString(const std::string& str_request);
    std::string         ParseToString() const;

    void                SetHeader(const std::string& h_name, const std::string& h_val);
    std::string         GetHeader(const std::string& h_name) const;
    HeaderDelStatus     DeleteHeader(const std::string& h_name);

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
    ParamsDelStatus     DeleteQueryParam(const std::string& p_name);
};

class Response {
 protected:
    ProtocolVersion __version;
    Headers         __headers;
    std::string     __body;
    int             __code;
 
 private:
    void                HeaderToString(std::string& str) const;

 public:
    // explicit
    // Response(const std::string& str_response);
    Response(int code, const std::string& body);

    ParseError          ParseFromString(const std::string& str_response);
    std::string         ParseToString() const;

    void                SetHeader(const std::string& h_name, const std::string& h_val);
    std::string         GetHeader(const std::string& h_name) const;
    HeaderDelStatus     DeleteHeader(const std::string& h_name);

    void                SetVersion(ProtocolVersion version);
    ProtocolVersion     GetVersion() const;

    void                SetBody(const std::string& body);
    std::string         GetBody() const;

    void                SetCode(int code);
    int                 GetCode() const;
};

}  // namespace ft

#endif  // HTTP_HTTP_H_
