#ifndef _GENERIC_COMPILER_H
#define _GENERIC_COMPILER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <linux/compiler_types.h>

/* Compiler detection and attributes */
#if defined(_MSC_VER)
  #define __forceinline __forceinline
  #define __noinline __declspec(noinline)
  #define barrier() _ReadWriteBarrier()
  #define likely(x) (x)
  #define unlikely(x) (x)
  #define unreachable() __assume(0)

#elif defined(__GNUC__) || defined(__clang__)
  #define __forceinline inline __attribute__((always_inline))
  #define __noinline __attribute__((noinline))
  #define barrier() __asm__ __volatile__("" ::: "memory")
  #define likely(x) __builtin_expect(!!(x), 1)
  #define unlikely(x) __builtin_expect(!!(x), 0)
  #define unreachable() do { barrier(); __builtin_unreachable(); } while (0)

#else
  #define __forceinline inline
  #define __noinline
  #define barrier()
  #define likely(x) (x)
  #define unlikely(x) (x)
  #define unreachable() do {} while (0)
#endif

#define likely_notrace(x) likely(x)
#define unlikely_notrace(x) unlikely(x)

#define prevent_tail_call_optimization() barrier()

#define RELOC_HIDE(ptr, off) \
  ({ uintptr_t __ptr = (uintptr_t)(ptr); (typeof(ptr)) (__ptr + (off)); })

#define absolute_pointer(val) RELOC_HIDE((void *)(val), 0)

#if defined(__GNUC__) || defined(__clang__)
  #define OPTIMIZER_HIDE_VAR(var) __asm__ ("" : "+r" (var))
#else
  #define OPTIMIZER_HIDE_VAR(var) (void)(var)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define __is_constexpr(x) \
    (sizeof(int) == sizeof(*(1 ? ((void *)((long)(x) * 0l)) : (int *)1)))
  #define const_true(x) __builtin_choose_expr(__is_constexpr(x), x, false)
#else
  #define __is_constexpr(x) (0)
  #define const_true(x) (false)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define is_signed_type(type) (((type)(-1)) < (__force type)1)
  #define is_unsigned_type(type) (!is_signed_type(type))
#else
  #define is_signed_type(type) (0)
  #define is_unsigned_type(type) (1)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define __BUILD_BUG_ON_ZERO_MSG(e, msg, ...) ((int)sizeof(struct {_Static_assert(!(e), msg);}))
#else
  #define __BUILD_BUG_ON_ZERO_MSG(e, msg, ...) (0)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define __is_array(a) (!__builtin_types_compatible_p(typeof(a), typeof(&(a)[0])))
  #define __must_be_array(a) __BUILD_BUG_ON_ZERO_MSG(!__is_array(a), "must be array")
  #define __is_byte_array(a) (__is_array(a) && sizeof((a)[0]) == 1)
  #define __must_be_byte_array(a) __BUILD_BUG_ON_ZERO_MSG(!__is_byte_array(a), "must be byte array")
#else
  #define __is_array(a) (0)
  #define __must_be_array(a) (0)
  #define __is_byte_array(a) (0)
  #define __must_be_byte_array(a) (0)
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define TYPEOF_UNQUAL(exp) typeof(exp)
#else
  #define TYPEOF_UNQUAL(exp) void*
#endif

#define data_race(expr) (expr)
#define __annotate_jump_table
#define KENTRY(sym)
#define __ADDRESSABLE(sym)
#define KCFI_REFERENCE(sym)
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)
#define ARCH_SEL(a, b) a

static inline void *offset_to_ptr(const int *off) {
    return (void *)((uintptr_t)off + *off);
}

#if defined(__GNUC__) || defined(__clang__)
  #define __section(name) __attribute__((section(name)))
  #define __used __attribute__((used))
  #define noinline __attribute__((noinline))
  #define __cold __attribute__((cold))
  #define __latent_entropy
  #define notrace
  #define __ADDRESSABLE __used
#else
  #define __section(name)
  #define __used
  #define noinline
  #define __cold
  #define __latent_entropy
  #define notrace
  #define __ADDRESSABLE
#endif

/* 
 * Define a packed struct that works both on MSVC and GCC/Clang
 * MSVC requires #pragma pack directives around the struct.
 */
#if defined(_MSC_VER)
  #pragma pack(push, 1)
#endif

struct __packed PackedData {
    uint8_t a;
    uint16_t b;
};

#if defined(_MSC_VER)
  #pragma pack(pop)
#endif

#endif /* _GENERIC_COMPILER_H */
