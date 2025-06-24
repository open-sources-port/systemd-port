#ifndef MY_COMPILER_TYPES_H
#define MY_COMPILER_TYPES_H

#include <linux/compiler_attributes.h>
#include <linux/asm/compiler.h>

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

/* Kernel-style annotations as no-ops for portability */
#define __force
#define __user
#define __kernel
#define __iomem
#define __percpu
#define __rcu
#define __must_hold(x)
#define __acquires(x)
#define __releases(x)

/* Attributes */
#if defined(_MSC_VER)
  #define __aligned(x) __declspec(align(x))
  #define __section(x)
  #define __maybe_unused
#else
  #define __aligned(x) __attribute__((aligned(x)))
  #define __section(x) __attribute__((section(x)))
  #define __maybe_unused __attribute__((unused))
#endif

/* typeof and type compatibility */
#if defined(__GNUC__) || defined(__clang__)
  #define my_typeof(x) typeof(x)
  #define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#else
  #define my_typeof(x) void *
  #define __same_type(a, b) 0
#endif

#endif // MY_COMPILER_TYPES_H
