#ifndef NETLIB_IO_ERRORS_H_
#define NETLIB_IO_ERRORS_H_

#include "common/error.h"

namespace IO {

enum ErrorCode {
    NET_NO_ERR = 10000,
    NET_SYSTEM_ERR,
    NET_BAD_FAMILY_ERR
};

}  // namespace IO

#endif  // NETLIB_IO_ERRORS_H_
