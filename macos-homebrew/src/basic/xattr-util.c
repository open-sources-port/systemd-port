/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <sys_compat/errno.h>
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

/*
 * macOS-only implementation
 *
 * flags:
 *   AT_SYMLINK_NOFOLLOW -> XATTR_NOFOLLOW
 *
 * Unsupported:
 *   AT_EMPTY_PATH
 */
int listxattr_at_malloc(int fd, const char *path, int flags, char **ret) {
        ssize_t size;
        char *buf = NULL;
        int options = 0;

        assert(ret);

        *ret = NULL;

        /* Validate arguments */
        if (fd < 0 && !path)
                return -EINVAL;

#ifdef AT_EMPTY_PATH
        if (flags & AT_EMPTY_PATH)
                return -EOPNOTSUPP;
#endif

#ifdef AT_SYMLINK_NOFOLLOW
        if (flags & AT_SYMLINK_NOFOLLOW)
                options |= XATTR_NOFOLLOW;
#endif

        /* Step 1: query size */
        if (path) {
                size = listxattr(path, NULL, 0, options);
        } else {
                size = flistxattr(fd, NULL, 0, options);
        }

        if (size < 0)
                return -errno;

        /* No xattrs */
        if (size == 0) {
                *ret = strdup("");
                if (!*ret)
                        return -ENOMEM;
                return 0;
        }

        /* Step 2: allocate */
        buf = malloc((size_t) size);
        if (!buf)
                return -ENOMEM;

        /* Step 3: fetch list */
        if (path) {
                size = listxattr(path, buf, (size_t) size, options);
        } else {
                size = flistxattr(fd, buf, (size_t) size, options);
        }

        if (size < 0) {
                free(buf);
                return -errno;
        }

        *ret = buf;
        return 0;
}

int getxattr_at_malloc(int fd, const char *path, const char *name, int flags, char **ret) {
    if (!name || !ret)
        return -EINVAL;

    *ret = NULL;
    ssize_t size;

    if (path) {
        // getxattr on a path
        size = getxattr(path, name, NULL, 0, 0, flags);
    } else if (fd >= 0) {
        // fgetxattr on a file descriptor
        size = fgetxattr(fd, name, NULL, 0, 0, flags);
    } else {
        return -EINVAL;
    }

    if (size < 0)
        return -errno;

    // allocate memory
    char *buf = malloc(size + 1); // +1 for safety (null-terminate)
    if (!buf)
        return -ENOMEM;

    ssize_t ret_size;
    if (path) {
        ret_size = getxattr(path, name, buf, size, 0, flags);
    } else {
        ret_size = fgetxattr(fd, name, buf, size, 0, flags);
    }

    if (ret_size < 0) {
        free(buf);
        return -errno;
    }

    buf[ret_size] = '\0'; // null terminate just in case
    *ret = buf;

    return ret_size;
}

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
