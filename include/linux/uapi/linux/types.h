#ifndef CROSS_PLATFORM_UAPI_LINUX_TYPES_H
#define CROSS_PLATFORM_UAPI_LINUX_TYPES_H

#include <stdint.h>

/* Bitwise attribute for sparse checker compatibility (noop if unsupported) */
#ifndef __CHECKER__
#define __bitwise
#else
#define __bitwise __attribute__((bitwise))
#endif

/* Basic fixed-width integer types */
typedef int8_t    __s8;
typedef uint8_t   __u8;

typedef int16_t   __s16;
typedef uint16_t  __u16;

typedef int32_t   __s32;
typedef uint32_t  __u32;

typedef int64_t   __s64;
typedef uint64_t  __u64;

/* Bitwise endianness-marked types */

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;

typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;

typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

/* Bitwise checksum types */
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

/* Aligned 64-bit types (8-byte aligned) */
#if defined(__GNUC__) || defined(__clang__)
#define __aligned_u64 __u64 __attribute__((aligned(8)))
#define __aligned_s64 __s64 __attribute__((aligned(8)))
#define __aligned_be64 __be64 __attribute__((aligned(8)))
#define __aligned_le64 __le64 __attribute__((aligned(8)))
#else
/* fallback without attribute */
#define __aligned_u64 __u64
#define __aligned_s64 __s64
#define __aligned_be64 __be64
#define __aligned_le64 __le64
#endif

/* Bitwise poll type */
typedef unsigned __bitwise __poll_t;

#endif /* CROSS_PLATFORM_UAPI_LINUX_TYPES_H */
