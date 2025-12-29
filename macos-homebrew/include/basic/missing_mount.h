/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <sys/mount.h>
#include <stdio.h>
#include <sys_compat/errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* dab741e0e02bd3c4f5e2e97be74b39df2523fc6e (5.10) */
#ifndef MS_NOSYMFOLLOW
#define MS_NOSYMFOLLOW 256
#endif

#define MS_RDONLY 0x1      /* stub */
#define MS_NODEV  0x2      /* stub */
#define LO_FLAGS_PARTSCAN 0 /* stub */

#define MS_BIND		4096
#define MS_MOVE		8192
#define MS_REC		16384
/* War is peace. Verbosity is silence. */
#define MS_VERBOSE	32768

#define LO_FLAGS_AUTOCLEAR   0
#define LO_FLAGS_READ_ONLY   0
#define LO_FLAGS_DIRECT_IO   0

#define LOOP_SET_STATUS_SETTABLE_FLAGS 0
#define LOOP_CTL_GET_FREE 0
#define LOOP_CTL_REMOVE   0

#ifndef MS_NOSUID
#define MS_NOSUID 0
#endif
#ifndef MS_NOEXEC
#define MS_NOEXEC 0
#endif

#ifndef MS_STRICTATIME
#define MS_STRICTATIME 0
#endif

/**
 * Mount a filesystem on macOS given a target file descriptor.
 * Linux-only filesystems like proc/sysfs/tmpfs are skipped.
 */
static inline int mount_mac(const char *filesystemtype, int target_fd, int flags, void *data) {
    char target_path[PATH_MAX];

    // Get the path from the file descriptor
    if (fcntl(target_fd, F_GETPATH, target_path) < 0) {
        perror("fcntl(F_GETPATH)");
        return -1;
    }

    // Skip Linux-only filesystems
    if (strcmp(filesystemtype, "proc") == 0 ||
        strcmp(filesystemtype, "sysfs") == 0 ||
        strcmp(filesystemtype, "tmpfs") == 0) 
    {
        fprintf(stderr, "Warning: filesystem '%s' not supported on macOS, skipping %s\n",
                filesystemtype, target_path);
        return 0; // emulate success
    }

    // macOS mount: mount(type, dir, flags, data)
    if (mount(filesystemtype, target_path, flags, data) < 0) {
        perror("mount");
        return -1;
    }

    return 0;
}

// int iterate_mounts_mac() {
//     struct statfs *mntbuf;
//     int count = getmntinfo(&mntbuf, MNT_NOWAIT);
//     if (!count) {
//         perror("getmntinfo");
//         return -1;
//     }

//     for (int i = 0; i < count; i++) {
//         printf("Mounted FS: %s on %s (type %s)\n",
//                mntbuf[i].f_mntfromname,
//                mntbuf[i].f_mntonname,
//                mntbuf[i].f_fstypename);
//     }

//     return 0;
// }
