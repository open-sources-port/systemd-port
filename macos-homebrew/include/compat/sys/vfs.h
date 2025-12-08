/* compat/sys/vfs.h
 *
 * Linux <sys/vfs.h> compatibility layer for macOS / BSD.
 * Provides basic typedefs, statfs mapping, and common FS_* magic constants
 * used by systemd.
 */
#pragma once

#ifndef COMPAT_SYS_VFS_H
#define COMPAT_SYS_VFS_H

#include <sys/param.h>
#include <sys/mount.h>

/*
 * Linux defines:
 *     struct statfs
 * macOS defines:
 *     struct statfs   (but with different fields)
 *
 * Most user code only checks:
 *     f_type (filesystem magic)
 *
 * macOS does NOT provide f_type.
 * You can approximate by using f_fstypename hash or numeric mapping.
 *
 * For compatibility, we provide f_type and fill it with 0,
 * unless the caller provides an override.
 */

#ifndef __fsword_t
typedef long __fsword_t;
#endif

#ifndef FS_MAGIC_FALLBACK
#define FS_MAGIC_FALLBACK 0
#endif

/* Provide Linux-like struct with f_type */
struct linux_statfs {
    __fsword_t f_type;      /* filesystem type (magic) */
    __fsword_t f_bsize;
    fsblkcnt_t f_blocks;
    fsblkcnt_t f_bfree;
    fsblkcnt_t f_bavail;
    fsfilcnt_t f_files;
    fsfilcnt_t f_ffree;
    struct { int __val[2]; } f_fsid;
    __fsword_t f_namelen;
    __fsword_t f_frsize;
    __fsword_t f_flags;
    __fsword_t f_spare[4];
};

/* Magic numbers commonly used in Linux/systemd */
#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC 0x01021994
#endif
#ifndef EXT4_SUPER_MAGIC
#define EXT4_SUPER_MAGIC 0xEF53
#endif
#ifndef BTRFS_SUPER_MAGIC
#define BTRFS_SUPER_MAGIC 0x9123683E
#endif
#ifndef XFS_SUPER_MAGIC
#define XFS_SUPER_MAGIC 0x58465342
#endif
#ifndef NFS_SUPER_MAGIC
#define NFS_SUPER_MAGIC 0x6969
#endif

#endif /* COMPAT_SYS_VFS_H */
