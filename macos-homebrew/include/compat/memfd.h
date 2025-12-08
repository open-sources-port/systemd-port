#pragma once

#ifdef __APPLE__
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

// Fallback: use shm_open + ftruncate to mimic memfd_create
static inline int memfd_create(const char *name, unsigned int flags) {
    // Create a POSIX shared memory object
    char shm_name[256];
    if (!name) {
        errno = EINVAL;
        return -1;
    }
    snprintf(shm_name, sizeof(shm_name), "/%s", name);

    int fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd < 0)
        return -1;

    // Unlink immediately so it is removed when closed
    shm_unlink(shm_name);
    return fd;
}
#endif
