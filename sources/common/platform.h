#ifndef COMMON_PLATFORM_H_
#define COMMON_PLATFORM_H_

#ifdef __linux__

#include <cerrno>
#include <cstring>


typedef typeof(errno) errno_t;


inline bool ishexnumber(int c) {
    return memchr("0123456789ABCDEFabcdef", c, 22) != NULL;
}

#endif  // __linux__

#endif  // COMMON_PLATFORM_H_
