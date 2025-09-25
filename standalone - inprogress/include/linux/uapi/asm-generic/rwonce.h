#ifndef _GENERIC_RWONCE_PORTABLE_H
#define _GENERIC_RWONCE_PORTABLE_H

#include <stdint.h>
#include <stddef.h>

/*
 * Prevent compiler from optimizing or reordering memory access.
 * This does not imply atomicity or memory barriers.
 * Use with care for simple scalar types or pointers.
 */

#define READ_ONCE(x) \
    (*(volatile __typeof__(x) *)&(x))

#define WRITE_ONCE(x, val) \
    (*(volatile __typeof__(x) *)&(x) = (val))

/*
 * Optionally add barriers if you're dealing with concurrency across threads.
 * These are compiler barriers only.
 */
#if defined(_MSC_VER)
    #define barrier() _ReadWriteBarrier()
#elif defined(__GNUC__) || defined(__clang__)
    #define barrier() __asm__ __volatile__("" ::: "memory")
#else
    #define barrier() /* fallback */
#endif

#endif /* _GENERIC_RWONCE_PORTABLE_H */
