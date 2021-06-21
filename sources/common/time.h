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

inline timeval_t
msec_to_tv(u64 msec) {
    timeval_t tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = msec % 1000;
    return tv;
}

inline u64
tv_to_usec(const timeval_t& tv) {
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

inline u64
tv_to_msec(const timeval_t& tv) {
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

inline u64 now_ms() {
    timeval_t tv;
    gettimeofday(&tv, NULL);
    return tv_to_msec(tv);
}

#endif  // COMMON_TIME_H_
