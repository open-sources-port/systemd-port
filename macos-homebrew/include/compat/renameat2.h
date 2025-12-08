#pragma once

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>   // for rename()
#include <unistd.h>  // for access()
#include <fcntl.h>   // optional, for flags

/* Partial renameat2 emulation: only supports RENAME_NOREPLACE */
#ifndef RENAME_NOREPLACE
#define RENAME_NOREPLACE (1 << 0)
#endif

static inline int renameat2(int olddirfd, const char *oldpath,
                            int newdirfd, const char *newpath,
                            unsigned int flags)
{
    (void)olddirfd;
    (void)newdirfd;

    if (flags & RENAME_NOREPLACE) {
        if (access(newpath, F_OK) == 0) {
            errno = EEXIST;
            return -1;
        }
    }

    return rename(oldpath, newpath);
}
