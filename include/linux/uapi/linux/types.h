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

#if defined(__SIZEOF_INT128__)
    typedef __int128  __s128;
    typedef unsigned __int128 __u128;
#define HAS_INT128 1
#else
    // Fallback definition or error
    #define HAS_INT128 0
#endif

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

#define __FD_SETSIZE 1024

typedef struct {
    unsigned long fds_bits[__FD_SETSIZE / (8 * sizeof(unsigned long))];
} __kernel_fd_set;

/* Long types (platform-dependent) */
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || defined(__LP64__)
typedef uint64_t  __kernel_ulong_t;
typedef int64_t   __kernel_long_t;
#else
typedef uint32_t  __kernel_ulong_t;
typedef int32_t   __kernel_long_t;
#endif

/* Core kernel-like typedefs */
typedef __kernel_ulong_t __kernel_size_t;
typedef __kernel_long_t  __kernel_ssize_t;
typedef __kernel_long_t  __kernel_ptrdiff_t;
typedef __kernel_long_t  __kernel_clock_t;
typedef void*            __kernel_caddr_t;

typedef __u32 __kernel_uid32_t;
typedef __u32 __kernel_gid32_t;
typedef __u16 __kernel_uid16_t;
typedef __u16 __kernel_gid16_t;

typedef __u16 __kernel_old_uid_t;
typedef __u16 __kernel_old_gid_t;

typedef __kernel_long_t __kernel_pid_t;
typedef __kernel_long_t __kernel_off_t;
typedef __u64           __kernel_loff_t;
typedef __kernel_ulong_t __kernel_ino_t;
typedef __u32           __kernel_mode_t;
typedef __s32           __kernel_daddr_t;
typedef __s32           __kernel_key_t;
typedef __s32           __kernel_suseconds_t;
typedef __s32           __kernel_timer_t;
typedef __s32           __kernel_clockid_t;
typedef __s32           __kernel_mqd_t;

#endif /* CROSS_PLATFORM_UAPI_LINUX_TYPES_H */
