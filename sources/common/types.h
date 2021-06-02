#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <sys/types.h>
#include <cstdint>
#include <unistd.h>

typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u8;

typedef int64_t     i64;
typedef int32_t     i32;
typedef int16_t     i16;
typedef int8_t      i8;

typedef size_t      usize;
typedef ssize_t     isize;

typedef int             fd_t;
typedef struct timeval  timeval_t;

#endif  // COMMON_TYPES_H_
