#pragma once

#include <unistd.h>
#include <errno.h>

#if defined(__APPLE__)

/* Approximate pivot_root using chroot */
static inline int pivot_root(const char *new_root, const char *put_old) {
    (void)put_old;  // cannot simulate put_old on macOS

    if (chroot(new_root) != 0) {
        return -1; // errno is set by chroot
    }

    // Change working directory to new root
    if (chdir("/") != 0) {
        return -1;
    }

    return 0;
}

#endif
