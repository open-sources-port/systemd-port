#ifndef PORTABLE_BUILD_BUG_H
#define PORTABLE_BUILD_BUG_H

#include <linux/compiler.h>

/*
 * Portable BUILD_BUG_ON macro that triggers a compile-time error if the condition is true.
 * It uses:
 *  - _Static_assert if C11
 *  - static_assert if C++11
 *  - typedef with negative array size otherwise
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define BUILD_BUG_ON(cond) _Static_assert(!(cond), "BUILD_BUG_ON failed: " #cond)
#elif defined(__cplusplus) && __cplusplus >= 201103L
  #define BUILD_BUG_ON(cond) static_assert(!(cond), "BUILD_BUG_ON failed: " #cond)
#elif defined(_MSC_VER)
  /* MSVC before C++11: use __pragma to emit error */
  #define BUILD_BUG_ON(cond) __pragma(warning(push)) \
                             __pragma(warning(disable:4127)) /* conditional expression is constant */ \
                             typedef char build_bug_on_failed[(cond) ? -1 : 1]; \
                             __pragma(warning(pop))
#else
  /* Fallback for other compilers */
  #define BUILD_BUG_ON(cond) typedef char build_bug_on_failed[(cond) ? -1 : 1] __attribute__((unused))
#endif

/*
 * BUILD_BUG() triggers unconditional compile-time error if supported,
 * else fallback to empty or runtime error.
 */
#if defined(__has_attribute)
#  if __has_attribute(error)
#    define BUILD_BUG() __attribute__((error("BUILD_BUG triggered"))) static inline void build_bug_func(void) {}
#  else
#    define BUILD_BUG() ((void)0)
#  endif
#else
#  define BUILD_BUG() ((void)0)
#endif

/*
 * BUILD_BUG_ON_MSG(condition, msg)
 * Same as BUILD_BUG_ON but with a custom message if supported.
 * Falls back to BUILD_BUG_ON.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define BUILD_BUG_ON_MSG(cond, msg) _Static_assert(!(cond), msg)
#elif defined(__cplusplus) && __cplusplus >= 201103L
  #define BUILD_BUG_ON_MSG(cond, msg) static_assert(!(cond), msg)
#else
  #define BUILD_BUG_ON_MSG(cond, msg) BUILD_BUG_ON(cond)
#endif

#endif /* PORTABLE_BUILD_BUG_H */
