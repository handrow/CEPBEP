#ifndef COMMON_PLATFORM_H_
#define COMMON_PLATFORM_H_

#ifdef linux

#include <cerrno>
#include <cstring>


typedef int errno_t;


inline bool ishexnumber(int c) {
    return memchr("0123456789ABCDEFabcdef", c, 22) != NULL;
}

#endif  // linux

#endif  // COMMON_PLATFORM_H_
