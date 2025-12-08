// compat_macro.h
#pragma once

#include <stddef.h> // for offsetof
#include <assert.h> // for static_assert if needed

#ifndef _pure_
#define _pure_ __attribute__((pure))
#endif

#ifndef _warn_unused_result_
#define _warn_unused_result_ __attribute__((warn_unused_result))
#endif

#ifndef _const_
#define _const_ __attribute__((const))
#endif

#ifndef _PRINTF_
#if defined(__GNUC__) || defined(__clang__)
#define _printf_(fmtarg, firstvararg) __attribute__((format(printf, fmtarg, firstvararg)))
#else
#define _printf_(fmtarg, firstvararg)
#endif
#endif

// Branch prediction hints
#ifndef likely
#  define likely(x)   __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#  define unlikely(x) __builtin_expect(!!(x), 0)
#endif

// Alignment helpers
#ifndef ALIGN
#  define ALIGN(x, a)        (((x) + ((a) - 1)) & ~((a) - 1))
#endif
#ifndef IS_ALIGNED
#  define IS_ALIGNED(x, a)   (((x) & ((typeof(x))(a) - 1)) == 0)
#endif

// Get number of elements in a static array
#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

// Container_of macro: get parent struct from member pointer
#ifndef container_of
#  define container_of(ptr, type, member) ({                  \
        const typeof(((type *)0)->member) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type, member));    \
    })
#endif

// Min/Max macros
#ifndef min
#  define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
#  define max(x, y) ((x) > (y) ? (x) : (y))
#endif

// Compile-time assert (C11 static_assert or fallback)
#ifndef BUILD_BUG_ON
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#    define BUILD_BUG_ON(cond) _Static_assert(!(cond), "BUILD_BUG_ON failed")
#  else
#    define BUILD_BUG_ON(cond) extern int build_bug_on[(cond) ? -1 : 1]
#  endif
#endif

#ifndef DEFINE_TRIVIAL_CLEANUP_FUNC
#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func) \
    static inline void func##_cleanup(type *p) { \
        if (p && *p) \
            func(*p); \
    }
#endif

#ifndef assert_cc
#define assert_cc(expr) _Static_assert(expr, "assert_cc failed: " #expr)
#endif
