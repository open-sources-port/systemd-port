// compat_types.h
#pragma once

#ifdef __linux__
    // On Linux, just use the real kernel types
    #include <linux/types.h>
#else
    // On macOS, BSD, and others:
    #include <stdint.h>
    #include <sys/types.h>

    // Unsigned types
    typedef uint8_t   __u8;
    typedef uint16_t  __u16;
    typedef uint32_t  __u32;
    typedef uint64_t  __u64;

    // Signed types
    typedef int8_t    __s8;
    typedef int16_t   __s16;
    typedef int32_t   __s32;
    typedef int64_t   __s64;

    // Kernel-style size types (map to standard equivalents)
    typedef uintptr_t __uptr;   // not in Linux but sometimes useful
    typedef intptr_t  __sptr;

    // Aliases for compatibility with code expecting Linux types
    typedef uint64_t  __kernel_ulong_t;
    typedef int64_t   __kernel_long_t;

    // Fallbacks for common POSIX types
    typedef off_t     __kernel_off_t;
    typedef pid_t     __kernel_pid_t;
    typedef uid_t     __kernel_uid_t;
    typedef gid_t     __kernel_gid_t;
#endif
