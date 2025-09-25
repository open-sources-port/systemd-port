#ifndef PORTABLE_COMPILER_H
#define PORTABLE_COMPILER_H

/* Detect compiler */
#if defined(__clang__)
  #define COMPILER_CLANG 1
#elif defined(__GNUC__)
  #define COMPILER_GCC 1
#elif defined(_MSC_VER)
  #define COMPILER_MSVC 1
#else
  #define COMPILER_UNKNOWN 1
#endif

/* Inline */
#if COMPILER_MSVC
  #define INLINE __inline
#else
  #define INLINE inline
#endif

/* No return */
#if COMPILER_MSVC
  #define NORETURN __declspec(noreturn)
#else
  #define NORETURN __attribute__((noreturn))
#endif

/* Likely / Unlikely branch prediction hints */
#if COMPILER_GCC || COMPILER_CLANG
  #define LIKELY(x)   __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define LIKELY(x)   (x)
  #define UNLIKELY(x) (x)
#endif

/* Compiler barrier */
#if COMPILER_GCC || COMPILER_CLANG
  #define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#elif COMPILER_MSVC
  #include <intrin.h>
  #define COMPILER_BARRIER() _ReadWriteBarrier()
#else
  #define COMPILER_BARRIER() /* unknown */
#endif

/* Deprecated attribute */
#if COMPILER_MSVC
  #define DEPRECATED __declspec(deprecated)
#elif COMPILER_GCC || COMPILER_CLANG
  #define DEPRECATED __attribute__((deprecated))
#else
  #define DEPRECATED
#endif

/* Force inline */
#if COMPILER_MSVC
  #define FORCE_INLINE __forceinline
#elif COMPILER_GCC || COMPILER_CLANG
  #define FORCE_INLINE inline __attribute__((always_inline))
#else
  #define FORCE_INLINE inline
#endif

/* Align attribute */
#if COMPILER_MSVC
  #define ALIGN(x) __declspec(align(x))
#elif COMPILER_GCC || COMPILER_CLANG
  #define ALIGN(x) __attribute__((aligned(x)))
#else
  #define ALIGN(x)
#endif

#endif /* PORTABLE_COMPILER_H */
