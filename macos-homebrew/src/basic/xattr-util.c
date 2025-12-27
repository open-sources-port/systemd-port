/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/xattr.h>

#include "alloc-util.h"
#include "errno-util.h"
#include "fd-util.h"
#include <basic/macro.h>
#include <sys_compat/missing_syscall.h>
#include "sparse-endian.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "time-util.h"
#include "xattr-util.h"

#include <stdbool.h>

int fd_setcrtime(int fd, usec_t usec) {
    struct {
        struct timespec ts;
    } buf;

    struct attrlist al = {
        .bitmapcount = ATTR_BIT_MAP_COUNT,
        .commonattr  = ATTR_CMN_CRTIME,
    };

    if (fd < 0)
        return -EINVAL;

    if (usec == 0 || usec == (usec_t) -1)
        return -EINVAL;

    buf.ts.tv_sec  = (time_t) (usec / USEC_PER_SEC);
    buf.ts.tv_nsec = (long) ((usec % USEC_PER_SEC) * NSEC_PER_USEC);

    /*
     * fsetattrlist is the ONLY supported way to set birth time on macOS
     */
    if (fsetattrlist(fd, &al, &buf, sizeof(buf), 0) < 0)
        return -errno;

    return 0;
}

int fd_getcrtime_at(int fd, const char *name, int flags, usec_t *ret) {
    struct stat st;

    if (!ret)
        return -EINVAL;

    if (fd < 0 && !name)
        return -EINVAL;

    if (fstatat(fd, name, &st, flags) < 0)
        return -errno;

    /*
     * macOS native creation (birth) time
     */
    if (st.st_birthtimespec.tv_sec <= 0)
        return -ENODATA;

    *ret =
        (usec_t) st.st_birthtimespec.tv_sec * USEC_PER_SEC +
        (usec_t) st.st_birthtimespec.tv_nsec / NSEC_PER_USEC;

    return 0;
}

inline ssize_t getxattr_mac(const char *path, const char *name, void *value, size_t size, int follow_symlink) {
    return getxattr(path, name, value, size, 0, follow_symlink ? 0 : XATTR_NOFOLLOW);
}

inline ssize_t fgetxattr_mac(int fd, const char *name, void *value, size_t size) {
    return fgetxattr(fd, name, value, size, 0, 0);
}

inline int fsetxattr_mac(int fd, const char *name, const void *value, size_t size) {
    return fsetxattr(fd, name, value, size, 0, 0);
}

inline ssize_t listxattr_mac(const char *path, char *list, size_t size, int follow_symlink) {
    return listxattr(path, list, size, follow_symlink ? 0 : XATTR_NOFOLLOW);
}

inline ssize_t flistxattr_mac(int fd, char *list, size_t size) {
    return flistxattr(fd, list, size, 0);
}

#define lgetxattr(path, name, value, size) getxattr_mac(path, name, value, size, 0)
#define llistxattr(path, list, size) listxattr_mac(path, list, size, 0)
#define getxattr(path, name, value, size) getxattr_mac(path, name, value, size, 1)
#define listxattr(path, list, size) listxattr_mac(path, list, size, 1)
#define fgetxattr(fd, name, value, size) fgetxattr_mac(fd, name, value, size)
#define flistxattr(fd, list, size) flistxattr_mac(fd, list, size)
#define fsetxattr(fd, name, value, size) fsetxattr_mac(fd, name, value, size)
