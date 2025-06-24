#ifndef MY_COMPILER_TYPES_H
#define MY_COMPILER_TYPES_H

#include <linux/compiler_attributes.h>
#include <uapi/asm/compiler.h>

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#define __force
#define __user
#define __kernel
#define __iomem
#define __percpu
#define __rcu
#define __must_hold(x)
#define __acquires(x)
#define __releases(x)
#define __aligned(x) __attribute__((aligned(x)))
#define __section(x)
#define __maybe_unused __attribute__((unused))

#if defined(__GNUC__) || defined(__clang__)
# define my_typeof(x) typeof(x)
# define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#else
# define my_typeof(x) void *
# define __same_type(a, b) 0
#endif

#endif // MY_COMPILER_TYPES_H
