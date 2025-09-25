#ifndef CROSS_PLATFORM_POSIX_TYPES_H
#define CROSS_PLATFORM_POSIX_TYPES_H

#include <stdint.h>

/* File descriptor set constants */
#undef __NFDBITS
#define __NFDBITS  (8 * sizeof(unsigned long))

#undef __FD_SETSIZE
#define __FD_SETSIZE 1024

#undef __FDSET_LONGS
#define __FDSET_LONGS (__FD_SETSIZE / __NFDBITS)

#undef __FDELT
#define __FDELT(d) ((d) / __NFDBITS)

#undef __FDMASK
#define __FDMASK(d) (1UL << ((d) % __NFDBITS))

/* fd_set-like struct */
typedef struct {
    unsigned long fds_bits[__FDSET_LONGS];
} __kernel_fd_set;

/* Manipulation macros/functions for __kernel_fd_set */

static inline void __kernel_FD_ZERO(__kernel_fd_set *set) {
    for (int i = 0; i < __FDSET_LONGS; i++) {
        set->fds_bits[i] = 0UL;
    }
}

static inline void __kernel_FD_SET(int fd, __kernel_fd_set *set) {
    set->fds_bits[__FDELT(fd)] |= __FDMASK(fd);
}

static inline void __kernel_FD_CLR(int fd, __kernel_fd_set *set) {
    set->fds_bits[__FDELT(fd)] &= ~__FDMASK(fd);
}

static inline int __kernel_FD_ISSET(int fd, const __kernel_fd_set *set) {
    return (set->fds_bits[__FDELT(fd)] & __FDMASK(fd)) != 0;
}

/* Signal handler type */
typedef void (*__kernel_sighandler_t)(int);

/* IPC key and message queue descriptor types */
typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

/* Note:
 * - On Windows/macOS, use native signal and IPC APIs.
 * - This header provides Linux-kernel style types/macros only.
 */

#endif /* CROSS_PLATFORM_POSIX_TYPES_H */
