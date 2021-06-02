#include "common/types.h"
#include "common/error.h"

#include <cstdio>

enum ParseErrors {
    OK,
    BAD_HEADER,
    BAD_GUY
};

const char* ParseErrMessages[] = {
    "No error",
    "Bad header",
    "Bad ass"
};

struct HttpError: public Error {
    explicit HttpError(ParseErrors ec) : Error(ec, ParseErrMessages[ec]) {}
};

int main(int, char**) {
    errno = 9;
    SystemError write_err(errno);
    printf("%d: %s\n", write_err.errcode, write_err.message.c_str());

    HttpError http_err(BAD_HEADER);
    printf("%d: %s\n", http_err.errcode, http_err.message.c_str());
}
