#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#include <bits/stdint-uintn.h>
#else
#include <cstdint>
#endif

typedef uint64_t    UInt64;
typedef uint32_t    UInt32;
typedef uint16_t    UInt16;
typedef uint8_t     UInt8;

typedef int64_t     Int64;
typedef int32_t     Int32;
typedef int16_t     Int16;
typedef int8_t      Int8;

typedef size_t      USize;
typedef ssize_t     ISize;

typedef int             Fd;
typedef struct timeval  TimeVal;

template<typename Typo>
inline void AssignPtrSafely(Typo* pointer, const Typo& val) {
    if (pointer != NULL) {
        *pointer = val;
    }
}

#endif  // COMMON_TYPES_H_
