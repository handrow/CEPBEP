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

    Int32       ErrorCode;
    std::string Description;

    explicit Error(Int32 ec = ERR_OK, const std::string& msg = std::string()) : ErrorCode(ec), Description(msg) {}

    inline bool IsOk() const { return ErrorCode == ERR_OK; }
    inline bool IsError() const { return ErrorCode != ERR_OK; }
};

struct SystemError: public Error {
    explicit SystemError(errno_t errnoCode = ERR_OK) : Error(errnoCode, strerror(errnoCode)) {
    }

    inline Error Base() const {
        return Error(ErrorCode, Description);
    }
};

#endif  // COMMON_ERROR_H_
