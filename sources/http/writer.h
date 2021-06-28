#ifndef HTTP_WRITER_H_
#define HTTP_WRITER_H_

#include "common/types.h"
#include "http/http.h"

namespace Http {

class ResponseWriter {
 public:
    Headers& Header();
    void     Write(const std::string& body_portion);

    std::string Result() const;
};

}  // namespace Http

#endif  // HTTP_WRITER_H_
