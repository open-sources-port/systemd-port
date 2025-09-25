#ifndef _CROSS_PLATFORM_UAPI_ASM_GENERIC_INT_L64_H
#define _CROSS_PLATFORM_UAPI_ASM_GENERIC_INT_L64_H

#include <stdint.h>

#ifndef __ASSEMBLY__

typedef int8_t  __s8;
typedef uint8_t __u8;

typedef int16_t  __s16;
typedef uint16_t __u16;

typedef int32_t  __s32;
typedef uint32_t __u32;

/* 64-bit types: use long on platforms where long is 64-bit, else fallback */

#if defined(_WIN32) || defined(_WIN64)
/* Windows: long is 32-bit */
typedef int64_t  __s64;
typedef uint64_t __u64;
#else
/* Linux/macOS 64-bit: long is 64-bit */
typedef long           __s64;
typedef unsigned long  __u64;
#endif

#endif /* __ASSEMBLY__ */
#endif /* CROSS_PLATFORM_UAPI_ASM_GENERIC_INT_L64_H */
