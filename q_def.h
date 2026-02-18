#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint8_t   u8;
typedef int32_t   b32;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
typedef char      byte;
typedef ptrdiff_t size;
typedef size_t    usize;

#if CHAR_BIT != 8
#error "CHAR_BIT != 8"
#endif

#if __STDC_VERSION__ < 202000
#error "NEED C23"
#endif

#define countof(a) (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(a) (countof(a) - 1)
