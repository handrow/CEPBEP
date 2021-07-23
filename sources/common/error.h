#ifndef COMMON_ERROR_H_
#define COMMON_ERROR_H_

#include <cstring>
#include <cerrno>
#include <string>

#include "common/platform.h"
#include "common/types.h"

struct Error {
    enum CommonCodes {
        ERR_OK = 0
    };

    i32         errcode;
    std::string message;

    explicit Error(i32 ec = ERR_OK, const std::string& msg = std::string()) : errcode(ec), message(msg) {}

    inline bool IsOk() const { return errcode == ERR_OK; }
    inline bool IsError() const { return errcode != ERR_OK; }
};

struct SystemError: public Error {
    explicit SystemError(errno_t errno_code = ERR_OK) : Error(errno_code, strerror(errno_code)) {
    }

    inline Error Base() const {
        return Error(this->errcode, this->message);
    }
};

#endif  // COMMON_ERROR_H_
