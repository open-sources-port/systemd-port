/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __PORTABLE_COMPILER_ATTRIBUTES_H
#define __PORTABLE_COMPILER_ATTRIBUTES_H

#if defined(__GNUC__) || defined(__clang__)
  #define __alias(symbol)                 __attribute__((__alias__(#symbol)))
  #define __aligned(x)                    __attribute__((__aligned__(x)))
  #define __aligned_largest               __attribute__((__aligned__))
  #define __always_inline                 inline __attribute__((__always_inline__))
  #define __assume_aligned(a, ...)        __attribute__((__assume_aligned__(a, ##__VA_ARGS__)))
  #define __cleanup(func)                 __attribute__((__cleanup__(func)))
  #define __attribute_const__             __attribute__((__const__))
  #define __compiletime_error(msg)        __attribute__((__error__(msg)))
  #define __compiletime_warning(msg)      __attribute__((__warning__(msg)))
  #define __deprecated                    __attribute__((__deprecated__))
  #define __designated_init               __attribute__((__designated_init__))
  #define __flatten                       __attribute__((__flatten__))
  #define __gnu_inline                    __attribute__((__gnu_inline__))
  #define __malloc                        __attribute__((__malloc__))
  #define __mode(x)                       __attribute__((__mode__(x)))
  #define __noinline                      __attribute__((__noinline__))
  #define __noreturn                      __attribute__((__noreturn__))
  #define __packed                        __attribute__((__packed__))
  #define __pure                          __attribute__((__pure__))
  #define __section(x)                    __attribute__((__section__(x)))
  #define __used                          __attribute__((__used__))
  #define __unused                        __attribute__((__unused__))
  #define __always_unused                 __attribute__((__unused__))
  #define __maybe_unused                  __attribute__((__unused__))
  #define __always_used                   __used __maybe_unused
  #define __must_check                    __attribute__((__warn_unused_result__))
  #define __weak                          __attribute__((__weak__))
  #define __printf(a, b)                  __attribute__((__format__(printf, a, b)))
  #define __scanf(a, b)                   __attribute__((__format__(scanf, a, b)))
  #define __no_stack_protector            __attribute__((__no_stack_protector__))
  #define __fix_address                   __noinline __attribute__((__noclone__))
  #define __noclone                       __attribute__((__noclone__))

  #if __has_attribute(__fallthrough__)
    #define fallthrough                  __attribute__((__fallthrough__))
  #else
    #define fallthrough                  do {} while (0) /* fallthrough */
  #endif

#else /* non-GCC/Clang, e.g., MSVC or unknown compiler */

  #define __alias(symbol)
  #define __aligned(x)
  #define __aligned_largest
  #define __always_inline                inline
  #define __assume_aligned(a, ...)
  #define __cleanup(func)
  #define __attribute_const__
  #define __compiletime_error(msg)
  #define __compiletime_warning(msg)
  #define __deprecated                   __declspec(deprecated)
  #define __designated_init
  #define __flatten
  #define __gnu_inline
  #define __malloc
  #define __mode(x)
  #define __noinline                     __declspec(noinline)
  #define __noreturn                     __declspec(noreturn)
  #define __packed
  #define __pure
  #define __section(x)
  #define __used
  #define __unused
  #define __always_unused
  #define __maybe_unused
  #define __always_used
  #define __must_check
  #define __weak
  #define __printf(a, b)
  #define __scanf(a, b)
  #define __no_stack_protector
  #define __fix_address
  #define __noclone
  #define fallthrough                    /* fallthrough */

#endif

#endif /* __PORTABLE_COMPILER_ATTRIBUTES_H */
