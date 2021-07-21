#ifndef COMMON_TIME_H_
#define COMMON_TIME_H_

#include <sys/time.h>
#include <ctime>
#include <string>

#include "common/types.h"

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

inline struct timeval
GetTimeOfDay() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

inline std::string
FormatTimeToStr(const std::string& format, const std::tm& tm, usize buff_size = 512) {
    std::string buff;
    usize len;

    do {
        buff.resize(buff_size);
        len = strftime(const_cast<char *>(buff.data()),
                       buff_size,
                       format.c_str(),
                       &tm);
        buff_size <<= 1;
    } while (len == 0);

    buff.resize(len);

    return buff;
}

inline std::string
FormatTimeToStr(const std::string& format, time_t sec, usize BUFF_SZ = 512) {
    std::tm tm;
    localtime_r(&sec, &tm);
    return FormatTimeToStr(format, tm, BUFF_SZ);
}

inline std::string
FormatTimeToStr(const std::string& format, const timeval_t& tv, usize BUFF_SZ = 512) {
    std::tm tm;
    localtime_r(&(tv.tv_sec), &tm);
    return FormatTimeToStr(format, tm, BUFF_SZ);
}

#endif  // COMMON_TIME_H_
