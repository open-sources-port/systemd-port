#pragma once
/* compat/sys/statfs.h
 *
 * Linux sys/statfs.h compatibility for macOS
 */

#ifdef __APPLE__

#include <sys/mount.h>
#include <sys/types.h>
#include <stddef.h>

/* Linux-like statfs structure */
struct linux_statfs {
    uint32_t f_type;
    uint32_t f_bsize;
    uint64_t f_blocks;
    uint64_t f_bfree;
    uint64_t f_bavail;
    uint64_t f_files;
    uint64_t f_ffree;
    uint64_t f_fsid;
    uint32_t f_namelen;
    uint32_t f_frsize;
};

static inline int linux_statfs(const char *path, struct linux_statfs *buf) {
    struct statfs st;     // << macOS structure, NOT linux_statfs

    if (statfs(path, &st) != 0)
        return -1;

    buf->f_type   = (uint32_t)st.f_type;
    buf->f_bsize  = (uint32_t)st.f_bsize;
    buf->f_blocks = st.f_blocks;
    buf->f_bfree  = st.f_bfree;
    buf->f_bavail = st.f_bavail;
    buf->f_files  = st.f_files;
    buf->f_ffree  = st.f_ffree;

    // macOS fsid_t = struct { int32_t val[2]; }
    uint32_t hi = (uint32_t)st.f_fsid.val[0];
    uint32_t lo = (uint32_t)st.f_fsid.val[1];
    buf->f_fsid = ((uint64_t)hi << 32) | lo;

    // macOS has no f_namelen; 1024 is safe limit
    buf->f_namelen = 1024;

    // macOS has no frsize; use block size
    buf->f_frsize = st.f_bsize;

    return 0;
}

#endif /* __APPLE__ */
