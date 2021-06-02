#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

#include <cerrno>
#include <string>

#include "common/types.h"

struct Error {
    enum CommonCodes {
        ERR_OK = 0
    };

    i32         errcode;
    std::string message;

    explicit Error(i32 ec, const std::string& msg = std::string()) : errcode(ec), message(msg) {}

    inline bool IsOk() const { return errcode == ERR_OK; }
    inline bool IsError() const { return errcode != ERR_OK; }
};

struct SystemError: public Error {
    enum { ERRNO_MSG_BUFF_SZ = 1024 };

    explicit SystemError(errno_t errno_code) : Error(errno_code) {
        char errno_msg_buffer[ERRNO_MSG_BUFF_SZ];
        strerror_r(errno_code, errno_msg_buffer, sizeof(errno_msg_buffer));
        this->message = std::string(reinterpret_cast<const char *>(errno_msg_buffer));
    }

    inline Error Base() const {
        return Error(this->errcode, this->message);
    }
};

#endif  // COMMON_ERROR_H_
