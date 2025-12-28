/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include "alloc-util.h"
#include "dirent-util.h"
#include "fd-util.h"
#include "fileio.h"
#include <sys_compat/missing_syscall.h>
#include <sys_compat/limits.h>
#include "mountpoint-util.h"
#include "recurse-dir.h"
#include "sort-util.h"

// Define flags if not defined
#define DEFAULT_RECURSION_MAX 100

static int sort_func(struct dirent * const *a, struct dirent * const *b) {
        return strcmp((*a)->d_name, (*b)->d_name);
}

static bool ignore_dirent(const struct dirent *de, RecurseDirFlags flags) {
        assert(de);

        /* Depending on flag either ignore everything starting with ".", or just "." itself and ".." */

        return FLAGS_SET(flags, RECURSE_DIR_IGNORE_DOT) ?
                de->d_name[0] == '.' :
                dot_or_dot_dot(de->d_name);
}

int readdir_all(int dir_fd,
                RecurseDirFlags flags,
                DirectoryEntries **ret) {

        _cleanup_free_ DirectoryEntries *de = NULL;
        struct dirent *entry;
        DirectoryEntries *nde;
        size_t add, sz, j;

        assert(dir_fd >= 0);

        /* Allocate initial DirectoryEntries structure */
        de = malloc(offsetof(DirectoryEntries, buffer) + DIRENT_SIZE_MAX * 8);
        if (!de)
                return -ENOMEM;

        de->buffer_size = 0;

        DIR *dir = fdopendir(dup(dir_fd));
        if (!dir) {
                free(de);
                return -errno;
        }

        /* Read all entries into buffer */
        while ((entry = readdir(dir)) != NULL) {
                if (ignore_dirent(entry, flags))
                        continue;

                if (de->buffer_size + sizeof(struct dirent) > DIRENT_SIZE_MAX * 8) {
                        nde = realloc(de, offsetof(DirectoryEntries, buffer) + de->buffer_size + sizeof(struct dirent));
                        if (!nde) {
                                closedir(dir);
                                free(de);
                                return -ENOMEM;
                        }
                        de = nde;
                }

                memcpy(de->buffer + de->buffer_size, entry, sizeof(struct dirent));
                de->buffer_size += sizeof(struct dirent);
        }

        closedir(dir);

        /* Count valid entries */
        de->n_entries = 0;
        FOREACH_DIRENT_IN_BUFFER(entry, de->buffer, de->buffer_size) {
                if (ignore_dirent(entry, flags))
                        continue;
                de->n_entries++;
        }

        /* Allocate entries array */
        sz = ALIGN(offsetof(DirectoryEntries, buffer) + de->buffer_size);
        add = sizeof(struct dirent*) * de->n_entries;
        if (add > SIZE_MAX - add) {
                free(de);
                return -ENOMEM;
        }

        nde = realloc(de, sz + add);
        if (!nde) {
                free(de);
                return -ENOMEM;
        }

        de = nde;
        de->entries = (struct dirent**) ((uint8_t*) de + ALIGN(offsetof(DirectoryEntries, buffer) + de->buffer_size));

        /* Fill entries array */
        j = 0;
        FOREACH_DIRENT_IN_BUFFER(entry, de->buffer, de->buffer_size) {
                if (ignore_dirent(entry, flags))
                        continue;
                de->entries[j++] = entry;
        }

        if (FLAGS_SET(flags, RECURSE_DIR_SORT))
                typesafe_qsort(de->entries, de->n_entries, sort_func);

        if (ret)
                *ret = TAKE_PTR(de);

        return 0;
}

#ifndef RECURSE_DIR_FOLLOW_SYMLINKS
#define RECURSE_DIR_FOLLOW_SYMLINKS (1 << 0)
#endif

int recurse_dir(int dir_fd, const char *path, unsigned statx_mask, unsigned n_depth_max,
                RecurseDirFlags flags, recurse_dir_func_t func, void *userdata) {

    if (n_depth_max == 0)
        return 0;

    DIR *dir = opendir(path);
    if (!dir)
        return -errno;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        struct stat st;
        int ret = fstatat(dir_fd, entry->d_name, &st,
                          (flags & RECURSE_DIR_FOLLOW_SYMLINKS) ? 0 : AT_SYMLINK_NOFOLLOW);
        if (ret < 0) {
            closedir(dir);
            return -errno;
        }

        // Call user callback (macOS expects 4 arguments)
        int r = func(dir_fd, path, entry->d_name, &st, flags, n_depth_max, userdata);
        if (r != 0) {
            closedir(dir);
            return r;
        }

        // Recurse into subdirectories
        if (S_ISDIR(st.st_mode)) {
            char child_path[PATH_MAX];
            snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->d_name);
            r = recurse_dir(dir_fd, child_path, statx_mask, n_depth_max - 1, flags, func, userdata);
            if (r != 0) {
                closedir(dir);
                return r;
            }
        }
    }

    closedir(dir);
    return 0;
}

int recurse_dir_at(
                int atfd,
                const char *path,
                unsigned statx_mask,
                unsigned n_depth_max,
                RecurseDirFlags flags,
                recurse_dir_func_t func,
                void *userdata) {

        _cleanup_close_ int fd = -1;

        assert(atfd >= 0 || atfd == AT_FDCWD);
        assert(func);

        fd = openat(atfd, path ?: ".", O_DIRECTORY|O_CLOEXEC);
        if (fd < 0)
                return -errno;

        return recurse_dir(fd, path, statx_mask, n_depth_max, flags, func, userdata);
}
