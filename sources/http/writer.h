#ifndef HTTP_WRITER_H_
#define HTTP_WRITER_H_

#include "common/types.h"
#include "http/http.h"

namespace Http {

class ResponseWriter {
 public:
    void            Reset();

    /**
     * Returns reference to current response Headers class.
     */
    Headers&        Header();
    const Headers&  Header() const;

    /**
     * Adds more bytes to entity-body
     */
    void            Write(const std::string& body_appendix);

    /**
     * Flushes inner buffers and returns raw represetation of a response.
     * Headers to set:
     * - Set Content-Length as size of the entity-body (ALWAYS)
     * - Set Date header as time when it was stringified (ALWAYS)
     */
    std::string     SendToString(int code, ProtocolVersion ver = HTTP_1_1);

    bool            HasBody() const; 

 protected:
    Headers         __hdrs;
    std::string     __body;
};

}  // namespace Http

#endif  // HTTP_WRITER_H_
