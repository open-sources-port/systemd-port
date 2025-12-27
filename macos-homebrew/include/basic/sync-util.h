/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

int fsync_directory_of_file(int fd);
int fsync_full(int fd);

int fsync_path_at(int at_fd, const char *path);
int fsync_parent_at(int at_fd, const char *path);
int fsync_path_and_parent_at(int at_fd, const char *path);

int syncfs_path(int at_fd, const char *path);

static inline int syncfs(int fd) {
    if (fd < 0)
            return 0;

    #ifdef F_FULLFSYNC
            if (fcntl(fd, F_FULLFSYNC) == 0)
                    return 0;
    #endif

    return fsync(fd);
}