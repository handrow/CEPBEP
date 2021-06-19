#ifndef COMMON_TIME_H_
#define COMMON_TIME_H_

#include "types.h"
#include <sys/time.h>

inline timeval_t
usec_to_tv(u64 usec) {
    timeval_t tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;
    return tv;
}

inline long
tv_to_usec(timeval_t tv) {
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

#endif  // COMMON_TIME_H_
