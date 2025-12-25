/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <compat/errno.h>
#include <sys_compat/missing_syscall.h>
#include <fundamental/macro-fundamental.h>
#include <basic/macro.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/loop.h>
#include <linux/magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "alloc-util.h"
#include "btrfs-util.h"
#include "chase-symlinks.h"
#include "chattr-util.h"
#include "copy.h"
#include "dirent-util.h"
#include "discover-image.h"
#include "dissect-image.h"
#include "env-file.h"
#include "env-util.h"
#include "fd-util.h"
#include "fs-util.h"
#include "hashmap.h"
#include "hostname-setup.h"
#include "id128-util.h"
#include "lockfile-util.h"
#include "log.h"
#include "loop-util.h"
#include "macro.h"
#include "mkdir.h"
#include "nulstr-util.h"
#include "os-util.h"
#include "path-util.h"
#include "rm-rf.h"
#include "stat-util.h"
#include "string-table.h"
#include "string-util.h"
#include "strv.h"
#include "time-util.h"
#include "utf8.h"
#include "xattr-util.h"

static const char* const image_search_path[_IMAGE_CLASS_MAX] = {
        [IMAGE_MACHINE] =   "/etc/machines\0"              /* only place symlinks here */
                            "/run/machines\0"              /* and here too */
                            "/var/lib/machines\0"          /* the main place for images */
                            "/var/lib/container\0"         /* legacy */
                            "/usr/local/lib/machines\0"
                            "/usr/lib/machines\0",

        [IMAGE_PORTABLE] =  "/etc/portables\0"             /* only place symlinks here */
                            "/run/portables\0"             /* and here too */
                            "/var/lib/portables\0"         /* the main place for images */
                            "/usr/local/lib/portables\0"
                            "/usr/lib/portables\0",

        [IMAGE_EXTENSION] = "/etc/extensions\0"             /* only place symlinks here */
                            "/run/extensions\0"             /* and here too */
                            "/var/lib/extensions\0"         /* the main place for images */
                            "/usr/local/lib/extensions\0"
                            "/usr/lib/extensions\0",
};

static Image *image_free(Image *i) {
        assert(i);

        free(i->name);
        free(i->path);

        free(i->hostname);
        strv_free(i->machine_info);
        strv_free(i->os_release);
        strv_free(i->extension_release);

        return mfree(i);
}

DEFINE_TRIVIAL_REF_UNREF_FUNC(Image, image, image_free);
DEFINE_HASH_OPS_WITH_VALUE_DESTRUCTOR(image_hash_ops, char, string_hash_func, string_compare_func,
                                      Image, image_unref);

static char **image_settings_path(Image *image) {
        _cleanup_strv_free_ char **l = NULL;
        const char *fn;
        unsigned i = 0;

        assert(image);

        l = new0(char*, 4);
        if (!l)
                return NULL;

        fn = strjoina(image->name, ".nspawn");

        FOREACH_STRING(s, "/etc/systemd/nspawn", "/run/systemd/nspawn") {
                l[i] = path_join(s, fn);
                if (!l[i])
                        return NULL;

                i++;
        }

        l[i] = file_in_same_dir(image->path, fn);
        if (!l[i])
                return NULL;

        return TAKE_PTR(l);
}

static char *image_roothash_path(Image *image) {
        const char *fn;

        assert(image);

        fn = strjoina(image->name, ".roothash");

        return file_in_same_dir(image->path, fn);
}

static int image_new(
                ImageType t,
                const char *pretty,
                const char *path,
                const char *filename,
                bool read_only,
                usec_t crtime,
                usec_t mtime,
                Image **ret) {

        _cleanup_(image_unrefp) Image *i = NULL;

        assert(t >= 0);
        assert(t < _IMAGE_TYPE_MAX);
        assert(pretty);
        assert(filename);
        assert(ret);

        i = new(Image, 1);
        if (!i)
                return -ENOMEM;

        *i = (Image) {
                .n_ref = 1,
                .type = t,
                .read_only = read_only,
                .crtime = crtime,
                .mtime = mtime,
                .usage = UINT64_MAX,
                .usage_exclusive = UINT64_MAX,
                .limit = UINT64_MAX,
                .limit_exclusive = UINT64_MAX,
        };

        i->name = strdup(pretty);
        if (!i->name)
                return -ENOMEM;

        i->path = path_join(path, filename);
        if (!i->path)
                return -ENOMEM;

        path_simplify(i->path);

        *ret = TAKE_PTR(i);

        return 0;
}

static int extract_pretty(const char *path, const char *suffix, char **ret) {
        _cleanup_free_ char *name = NULL;
        const char *p;
        size_t n;

        assert(path);
        assert(ret);

        p = last_path_component(path);
        n = strcspn(p, "/");

        name = strndup(p, n);
        if (!name)
                return -ENOMEM;

        if (suffix) {
                char *e;

                e = endswith(name, suffix);
                if (!e)
                        return -EINVAL;

                *e = 0;
        }

        if (!image_name_is_valid(name))
                return -EINVAL;

        *ret = TAKE_PTR(name);
        return 0;
}

static int image_update_quota(Image *i, int fd) {
        _cleanup_close_ int fd_close = -EBADF;
        int r;

        assert(i);

        if (IMAGE_IS_VENDOR(i) || IMAGE_IS_HOST(i))
                return -EROFS;

        if (i->type != IMAGE_SUBVOLUME)
                return -EOPNOTSUPP;

        if (fd < 0) {
                fd_close = open(i->path, O_CLOEXEC|O_NOCTTY|O_DIRECTORY);
                if (fd_close < 0)
                        return -errno;
                fd = fd_close;
        }

        r = btrfs_quota_scan_ongoing(fd);
        if (r < 0)
                return r;
        if (r > 0)
                return 0;

        BtrfsQuotaInfo quota;
        r = btrfs_subvol_get_subtree_quota_fd(fd, 0, &quota);
        if (r < 0)
                return r;

        i->usage = quota.referenced;
        i->usage_exclusive = quota.exclusive;
        i->limit = quota.referenced_max;
        i->limit_exclusive = quota.exclusive_max;

        return 1;
}

static int image_make(
                const char *pretty,
                int dfd,
                const char *path,
                const char *filename,
                const struct stat *st,
                Image **ret) {

    _cleanup_free_ char *pretty_buffer = NULL, *parent = NULL;
    struct stat stbuf;
    bool read_only;
    int r;

    assert(dfd >= 0 || dfd == AT_FDCWD);
    assert(path || dfd == AT_FDCWD);
    assert(filename);

    // If stat info not provided, get it
    if (!st) {
        if (fstatat(dfd, filename, &stbuf, 0) < 0)
            return -errno;
        st = &stbuf;
    }

    if (!path) {
        if (dfd == AT_FDCWD)
            (void) safe_getcwd(&parent);
        else
            (void) fd_get_path(dfd, &parent);
    }

    read_only =
        (path && path_startswith(path, "/usr")) ||
        (faccessat(dfd, filename, W_OK, AT_EACCESS) < 0 && errno == EROFS);

    // ------------------------
    // Block device branch (macOS)
    // ------------------------
    if (S_ISBLK(st->st_mode)) {
        _cleanup_close_ int block_fd = -1;
        uint64_t size = UINT64_MAX;

        if (!ret)
            return 0;

        if (!pretty) {
            r = extract_pretty(filename, NULL, &pretty_buffer);
            if (r < 0)
                return r;
            pretty = pretty_buffer;
        }

        // Open block device readonly
        block_fd = openat(dfd, filename, O_RDONLY | O_CLOEXEC | O_NONBLOCK | O_NOCTTY);
        if (block_fd < 0) {
            log_debug_errno(errno,
                            "Failed to open block device %s/%s, ignoring: %m",
                            path ?: strnull(parent), filename);
        } else {
            // Refresh stat info
            if (fstat(block_fd, &stbuf) < 0)
                return -errno;
            st = &stbuf;

            if (!S_ISBLK(st->st_mode))
                return -ENOTTY;

            // Check read-only using fcntl
            int flags = fcntl(block_fd, F_GETFL);
            if (flags < 0)
                log_debug_errno(errno,
                                "Failed to get flags for device %s/%s, ignoring: %m",
                                path ?: strnull(parent), filename);
            else if ((flags & O_ACCMODE) != O_WRONLY)
                read_only = true;

            // Get disk size using DKIOC
            uint64_t block_size = 0, block_count = 0;
            if (ioctl(block_fd, DKIOCGETBLOCKSIZE, &block_size) == -1)
                log_debug_errno(errno,
                                "Failed to get block size for %s/%s, ignoring: %m",
                                path ?: strnull(parent), filename);
            else if (ioctl(block_fd, DKIOCGETBLOCKCOUNT, &block_count) == -1)
                log_debug_errno(errno,
                                "Failed to get block count for %s/%s, ignoring: %m",
                                path ?: strnull(parent), filename);
            else
                size = block_size * block_count;

            block_fd = safe_close(block_fd);
        }

        r = image_new(IMAGE_BLOCK,
                      pretty,
                      path,
                      filename,
                      !(st->st_mode & 0222) || read_only,
                      0,
                      0,
                      ret);
        if (r < 0)
            return r;

        if (!IN_SET(size, 0, UINT64_MAX))
            (*ret)->usage = (*ret)->usage_exclusive =
            (*ret)->limit = (*ret)->limit_exclusive = size;

        return 0;
    }

    return -EMEDIUMTYPE;
}

int image_find(ImageClass class,
               const char *name,
               const char *root,
               Image **ret) {

        const char *path;
        int r;

        assert(class >= 0);
        assert(class < _IMAGE_CLASS_MAX);
        assert(name);

        /* There are no images with invalid names */
        if (!image_name_is_valid(name))
                return -ENOENT;

        NULSTR_FOREACH(path, image_search_path[class]) {
                _cleanup_free_ char *resolved = NULL;
                _cleanup_closedir_ DIR *d = NULL;
                struct stat st;
                int flags;

                r = chase_symlinks_and_opendir(path, root, CHASE_PREFIX_ROOT, &resolved, &d);
                if (r == -ENOENT)
                        continue;
                if (r < 0)
                        return r;

                /* As mentioned above, we follow symlinks on this fstatat(), because we want to permit people
                 * to symlink block devices into the search path. (For now, we disable that when operating
                 * relative to some root directory.) */
                flags = root ? AT_SYMLINK_NOFOLLOW : 0;
                if (fstatat(dirfd(d), name, &st, flags) < 0) {
                        _cleanup_free_ char *raw = NULL;

                        if (errno != ENOENT)
                                return -errno;

                        raw = strjoin(name, ".raw");
                        if (!raw)
                                return -ENOMEM;

                        if (fstatat(dirfd(d), raw, &st, flags) < 0) {
                                if (errno == ENOENT)
                                        continue;

                                return -errno;
                        }

                        if (!S_ISREG(st.st_mode))
                                continue;

                        r = image_make(name, dirfd(d), resolved, raw, &st, ret);

                } else {
                        if (!S_ISDIR(st.st_mode) && !S_ISBLK(st.st_mode))
                                continue;

                        r = image_make(name, dirfd(d), resolved, name, &st, ret);
                }
                if (IN_SET(r, -ENOENT, -EMEDIUMTYPE))
                        continue;
                if (r < 0)
                        return r;

                if (ret)
                        (*ret)->discoverable = true;

                return 1;
        }

        if (class == IMAGE_MACHINE && streq(name, ".host")) {
                r = image_make(".host", AT_FDCWD, NULL, empty_to_root(root), NULL, ret);
                if (r < 0)
                        return r;

                if (ret)
                        (*ret)->discoverable = true;

                return r;
        }

        return -ENOENT;
};

int image_from_path(const char *path, Image **ret) {

        /* Note that we don't set the 'discoverable' field of the returned object, because we don't check here whether
         * the image is in the image search path. And if it is we don't know if the path we used is actually not
         * overridden by another, different image earlier in the search path */

        if (path_equal(path, "/"))
                return image_make(".host", AT_FDCWD, NULL, "/", NULL, ret);

        return image_make(NULL, AT_FDCWD, NULL, path, NULL, ret);
}

int image_find_harder(ImageClass class, const char *name_or_path, const char *root, Image **ret) {
        if (image_name_is_valid(name_or_path))
                return image_find(class, name_or_path, root, ret);

        return image_from_path(name_or_path, ret);
}

int image_discover(
                ImageClass class,
                const char *root,
                Hashmap *h) {

        const char *path;
        int r;

        assert(class >= 0);
        assert(class < _IMAGE_CLASS_MAX);
        assert(h);

        NULSTR_FOREACH(path, image_search_path[class]) {
                _cleanup_free_ char *resolved = NULL;
                _cleanup_closedir_ DIR *d = NULL;

                r = chase_symlinks_and_opendir(path, root, CHASE_PREFIX_ROOT, &resolved, &d);
                if (r == -ENOENT)
                        continue;
                if (r < 0)
                        return r;

                FOREACH_DIRENT_ALL(de, d, return -errno) {
                        _cleanup_(image_unrefp) Image *image = NULL;
                        _cleanup_free_ char *truncated = NULL;
                        const char *pretty;
                        struct stat st;
                        int flags;

                        if (dot_or_dot_dot(de->d_name))
                                continue;

                        /* As mentioned above, we follow symlinks on this fstatat(), because we want to
                         * permit people to symlink block devices into the search path. */
                        flags = root ? AT_SYMLINK_NOFOLLOW : 0;
                        if (fstatat(dirfd(d), de->d_name, &st, flags) < 0) {
                                if (errno == ENOENT)
                                        continue;

                                return -errno;
                        }

                        if (S_ISREG(st.st_mode)) {
                                const char *e;

                                e = endswith(de->d_name, ".raw");
                                if (!e)
                                        continue;

                                truncated = strndup(de->d_name, e - de->d_name);
                                if (!truncated)
                                        return -ENOMEM;

                                pretty = truncated;
                        } else if (S_ISDIR(st.st_mode) || S_ISBLK(st.st_mode))
                                pretty = de->d_name;
                        else
                                continue;

                        if (!image_name_is_valid(pretty))
                                continue;

                        if (hashmap_contains(h, pretty))
                                continue;

                        r = image_make(pretty, dirfd(d), resolved, de->d_name, &st, &image);
                        if (IN_SET(r, -ENOENT, -EMEDIUMTYPE))
                                continue;
                        if (r < 0)
                                return r;

                        image->discoverable = true;

                        r = hashmap_put(h, image->name, image);
                        if (r < 0)
                                return r;

                        image = NULL;
                }
        }

        if (class == IMAGE_MACHINE && !hashmap_contains(h, ".host")) {
                _cleanup_(image_unrefp) Image *image = NULL;

                r = image_make(".host", AT_FDCWD, NULL, empty_to_root("/"), NULL, &image);
                if (r < 0)
                        return r;

                image->discoverable = true;

                r = hashmap_put(h, image->name, image);
                if (r < 0)
                        return r;

                image = NULL;
        }

        return 0;
}

int image_remove(Image *i) {
        _cleanup_(release_lock_file) LockFile global_lock = LOCK_FILE_INIT, local_lock = LOCK_FILE_INIT;
        _cleanup_strv_free_ char **settings = NULL;
        _cleanup_free_ char *roothash = NULL;
        int r;

        assert(i);

        if (IMAGE_IS_VENDOR(i) || IMAGE_IS_HOST(i))
                return -EROFS;

        settings = image_settings_path(i);
        if (!settings)
                return -ENOMEM;

        roothash = image_roothash_path(i);
        if (!roothash)
                return -ENOMEM;

        /* Make sure we don't interfere with a running nspawn */
        r = image_path_lock(i->path, LOCK_EX|LOCK_NB, &global_lock, &local_lock);
        if (r < 0)
                return r;

        switch (i->type) {

        case IMAGE_SUBVOLUME:

                /* Let's unlink first, maybe it is a symlink? If that works we are happy. Otherwise, let's get out the
                 * big guns */
                if (unlink(i->path) < 0) {
                        r = btrfs_subvol_remove(i->path, BTRFS_REMOVE_RECURSIVE|BTRFS_REMOVE_QUOTA);
                        if (r < 0)
                                return r;
                }

                break;

        case IMAGE_DIRECTORY:
                /* Allow deletion of read-only directories */
                (void) chattr_path(i->path, 0, FS_IMMUTABLE_FL, NULL);
                r = rm_rf(i->path, REMOVE_ROOT|REMOVE_PHYSICAL|REMOVE_SUBVOLUME);
                if (r < 0)
                        return r;

                break;

        case IMAGE_BLOCK:

                /* If this is inside of /dev, then it's a real block device, hence let's not touch the device node
                 * itself (but let's remove the stuff stored alongside it). If it's anywhere else, let's try to unlink
                 * the thing (it's most likely a symlink after all). */

                if (path_startswith(i->path, "/dev"))
                        break;

                _fallthrough_;
        case IMAGE_RAW:
                if (unlink(i->path) < 0)
                        return -errno;
                break;

        default:
                return -EOPNOTSUPP;
        }

        STRV_FOREACH(j, settings)
                if (unlink(*j) < 0 && errno != ENOENT)
                        log_debug_errno(errno, "Failed to unlink %s, ignoring: %m", *j);

        if (unlink(roothash) < 0 && errno != ENOENT)
                log_debug_errno(errno, "Failed to unlink %s, ignoring: %m", roothash);

        return 0;
}

static int rename_auxiliary_file(const char *path, const char *new_name, const char *suffix) {
        _cleanup_free_ char *rs = NULL;
        const char *fn;

        fn = strjoina(new_name, suffix);

        rs = file_in_same_dir(path, fn);
        if (!rs)
                return -ENOMEM;

        return rename_noreplace(AT_FDCWD, path, AT_FDCWD, rs);
}

int image_rename(Image *i, const char *new_name) {
        _cleanup_(release_lock_file) LockFile global_lock = LOCK_FILE_INIT, local_lock = LOCK_FILE_INIT, name_lock = LOCK_FILE_INIT;
        _cleanup_free_ char *new_path = NULL, *nn = NULL, *roothash = NULL;
        _cleanup_strv_free_ char **settings = NULL;
        unsigned file_attr = 0;
        int r;

        assert(i);

        if (!image_name_is_valid(new_name))
                return -EINVAL;

        if (IMAGE_IS_VENDOR(i) || IMAGE_IS_HOST(i))
                return -EROFS;

        settings = image_settings_path(i);
        if (!settings)
                return -ENOMEM;

        roothash = image_roothash_path(i);
        if (!roothash)
                return -ENOMEM;

        /* Make sure we don't interfere with a running nspawn */
        r = image_path_lock(i->path, LOCK_EX|LOCK_NB, &global_lock, &local_lock);
        if (r < 0)
                return r;

        /* Make sure nobody takes the new name, between the time we
         * checked it is currently unused in all search paths, and the
         * time we take possession of it */
        r = image_name_lock(new_name, LOCK_EX|LOCK_NB, &name_lock);
        if (r < 0)
                return r;

        r = image_find(IMAGE_MACHINE, new_name, NULL, NULL);
        if (r >= 0)
                return -EEXIST;
        if (r != -ENOENT)
                return r;

        switch (i->type) {

        case IMAGE_DIRECTORY:
                /* Turn of the immutable bit while we rename the image, so that we can rename it */
                (void) read_attr_path(i->path, &file_attr);

                if (file_attr & FS_IMMUTABLE_FL)
                        (void) chattr_path(i->path, 0, FS_IMMUTABLE_FL, NULL);

                _fallthrough_;
        case IMAGE_SUBVOLUME:
                new_path = file_in_same_dir(i->path, new_name);
                break;

        case IMAGE_BLOCK:

                /* Refuse renaming raw block devices in /dev, the names are picked by udev after all. */
                if (path_startswith(i->path, "/dev"))
                        return -EROFS;

                new_path = file_in_same_dir(i->path, new_name);
                break;

        case IMAGE_RAW: {
                const char *fn;

                fn = strjoina(new_name, ".raw");
                new_path = file_in_same_dir(i->path, fn);
                break;
        }

        default:
                return -EOPNOTSUPP;
        }

        if (!new_path)
                return -ENOMEM;

        nn = strdup(new_name);
        if (!nn)
                return -ENOMEM;

        r = rename_noreplace(AT_FDCWD, i->path, AT_FDCWD, new_path);
        if (r < 0)
                return r;

        /* Restore the immutable bit, if it was set before */
        if (file_attr & FS_IMMUTABLE_FL)
                (void) chattr_path(new_path, FS_IMMUTABLE_FL, FS_IMMUTABLE_FL, NULL);

        free_and_replace(i->path, new_path);
        free_and_replace(i->name, nn);

        STRV_FOREACH(j, settings) {
                r = rename_auxiliary_file(*j, new_name, ".nspawn");
                if (r < 0 && r != -ENOENT)
                        log_debug_errno(r, "Failed to rename settings file %s, ignoring: %m", *j);
        }

        r = rename_auxiliary_file(roothash, new_name, ".roothash");
        if (r < 0 && r != -ENOENT)
                log_debug_errno(r, "Failed to rename roothash file %s, ignoring: %m", roothash);

        return 0;
}

static int clone_auxiliary_file(const char *path, const char *new_name, const char *suffix) {
        _cleanup_free_ char *rs = NULL;
        const char *fn;

        fn = strjoina(new_name, suffix);

        rs = file_in_same_dir(path, fn);
        if (!rs)
                return -ENOMEM;

        return copy_file_atomic(path, rs, 0664, 0, 0, COPY_REFLINK);
}

int image_clone(Image *i, const char *new_name, bool read_only) {
        _cleanup_(release_lock_file) LockFile name_lock = LOCK_FILE_INIT;
        _cleanup_strv_free_ char **settings = NULL;
        _cleanup_free_ char *roothash = NULL;
        const char *new_path;
        int r;

        assert(i);

        if (!image_name_is_valid(new_name))
                return -EINVAL;

        settings = image_settings_path(i);
        if (!settings)
                return -ENOMEM;

        roothash = image_roothash_path(i);
        if (!roothash)
                return -ENOMEM;

        /* Make sure nobody takes the new name, between the time we
         * checked it is currently unused in all search paths, and the
         * time we take possession of it */
        r = image_name_lock(new_name, LOCK_EX|LOCK_NB, &name_lock);
        if (r < 0)
                return r;

        r = image_find(IMAGE_MACHINE, new_name, NULL, NULL);
        if (r >= 0)
                return -EEXIST;
        if (r != -ENOENT)
                return r;

        switch (i->type) {

        case IMAGE_SUBVOLUME:
        case IMAGE_DIRECTORY:
                /* If we can we'll always try to create a new btrfs subvolume here, even if the source is a plain
                 * directory. */

                new_path = strjoina("/var/lib/machines/", new_name);

                r = btrfs_subvol_snapshot(i->path, new_path,
                                          (read_only ? BTRFS_SNAPSHOT_READ_ONLY : 0) |
                                          BTRFS_SNAPSHOT_FALLBACK_COPY |
                                          BTRFS_SNAPSHOT_FALLBACK_DIRECTORY |
                                          BTRFS_SNAPSHOT_FALLBACK_IMMUTABLE |
                                          BTRFS_SNAPSHOT_RECURSIVE |
                                          BTRFS_SNAPSHOT_QUOTA);
                if (r >= 0)
                        /* Enable "subtree" quotas for the copy, if we didn't copy any quota from the source. */
                        (void) btrfs_subvol_auto_qgroup(new_path, 0, true);

                break;

        case IMAGE_RAW:
                new_path = strjoina("/var/lib/machines/", new_name, ".raw");

                r = copy_file_atomic(i->path, new_path, read_only ? 0444 : 0644, FS_NOCOW_FL, FS_NOCOW_FL, COPY_REFLINK|COPY_CRTIME);
                break;

        case IMAGE_BLOCK:
        default:
                return -EOPNOTSUPP;
        }

        if (r < 0)
                return r;

        STRV_FOREACH(j, settings) {
                r = clone_auxiliary_file(*j, new_name, ".nspawn");
                if (r < 0 && r != -ENOENT)
                        log_debug_errno(r, "Failed to clone settings %s, ignoring: %m", *j);
        }

        r = clone_auxiliary_file(roothash, new_name, ".roothash");
        if (r < 0 && r != -ENOENT)
                log_debug_errno(r, "Failed to clone root hash file %s, ignoring: %m", roothash);

        return 0;
}

int image_read_only(Image *i, bool b) {
    _cleanup_(release_lock_file) LockFile global_lock = LOCK_FILE_INIT;
    _cleanup_(release_lock_file) LockFile local_lock = LOCK_FILE_INIT;
    int r;

    assert(i);

    if (IMAGE_IS_VENDOR(i) || IMAGE_IS_HOST(i))
        return -EROFS;

    r = image_path_lock(i->path, 0 /*LOCK_EX|LOCK_NB*/, &global_lock, &local_lock);
    if (r < 0)
        return r;

    switch (i->type) {
    case IMAGE_DIRECTORY: {
        // Set immutable flag if requested (macOS uses UF_IMMUTABLE)
        struct stat st;
        if (stat(i->path, &st) < 0)
            return -errno;

        mode_t new_flags = b ? st.st_flags | UF_IMMUTABLE : st.st_flags & ~UF_IMMUTABLE;
        if (chflags(i->path, new_flags) < 0)
            return -errno;

        break;
    }

    case IMAGE_RAW: {
        struct stat st;
        if (stat(i->path, &st) < 0)
            return -errno;

        // chmod readonly or writable
        if (chmod(i->path, (st.st_mode & 0444) | (b ? 0000 : 0200)) < 0)
            return -errno;

        // Defrag if needed
        if (b)
            (void) btrfs_defrag(i->path); // keep as no-op on macOS if not Btrfs

        break;
    }

    case IMAGE_BLOCK: {
        _cleanup_close_ int fd = -1;
        struct stat st;

        fd = open(i->path, O_CLOEXEC | O_RDONLY | O_NONBLOCK | O_NOCTTY);
        if (fd < 0)
            return -errno;

        if (fstat(fd, &st) < 0)
            return -errno;

        if (!S_ISBLK(st.st_mode))
            return -ENOTTY;

        // macOS: use fcntl to check read-only; cannot force read-only on device
        if (b) {
            int flags = fcntl(fd, F_GETFL);
            if (flags >= 0 && (flags & O_ACCMODE) == O_WRONLY) {
                log_debug_errno(0, "Warning: cannot set block device %s read-only", i->path);
            }
        }

        fd = safe_close(fd);
        break;
    }

    default:
        return -EOPNOTSUPP;
    }

    i->read_only = b;
    return 0;
}

static void make_lock_dir(void) {
        (void) mkdir_p("/run/systemd/nspawn", 0755);
        (void) mkdir("/run/systemd/nspawn/locks", 0700);
}

int image_path_lock(const char *path, int operation, LockFile *global, LockFile *local) {
        _cleanup_free_ char *p = NULL;
        LockFile t = LOCK_FILE_INIT;
        struct stat st;
        bool exclusive;
        int r;

        assert(path);
        assert(global);
        assert(local);

        /* Locks an image path. This actually creates two locks: one "local" one, next to the image path
         * itself, which might be shared via NFS. And another "global" one, in /run, that uses the
         * device/inode number. This has the benefit that we can even lock a tree that is a mount point,
         * correctly. */

        if (!path_is_absolute(path))
                return -EINVAL;

        switch (operation & (LOCK_SH|LOCK_EX)) {
        case LOCK_SH:
                exclusive = false;
                break;
        case LOCK_EX:
                exclusive = true;
                break;
        default:
                return -EINVAL;
        }

        if (getenv_bool("SYSTEMD_NSPAWN_LOCK") == 0) {
                *local = *global = (LockFile) LOCK_FILE_INIT;
                return 0;
        }

        /* Prohibit taking exclusive locks on the host image. We can't allow this, since we ourselves are
         * running off it after all, and we don't want any images to manipulate the host image. We make an
         * exception for shared locks however: we allow those (and make them NOPs since there's no point in
         * taking them if there can't be exclusive locks). Strictly speaking these are questionable as well,
         * since it means changes made to the host might propagate to the container as they happen (and a
         * shared lock kinda suggests that no changes happen at all while it is in place), but it's too
         * useful not to allow read-only containers off the host root, hence let's support this, and trust
         * the user to do the right thing with this. */
        if (path_equal(path, "/")) {
                if (exclusive)
                        return -EBUSY;

                *local = *global = (LockFile) LOCK_FILE_INIT;
                return 0;
        }

        if (stat(path, &st) >= 0) {
                if (S_ISBLK(st.st_mode))
                        r = asprintf(&p, "/run/systemd/nspawn/locks/block-%d:%d", major(st.st_rdev), minor(st.st_rdev));
                else if (S_ISDIR(st.st_mode) || S_ISREG(st.st_mode))
                        r = asprintf(&p, "/run/systemd/nspawn/locks/inode-%lu:%lu", (unsigned long) st.st_dev, (unsigned long) st.st_ino);
                else
                        return -ENOTTY;
                if (r < 0)
                        return -ENOMEM;
        }

        /* For block devices we don't need the "local" lock, as the major/minor lock above should be
         * sufficient, since block devices are host local anyway. */
        if (!path_startswith(path, "/dev/")) {
                r = make_lock_file_for(path, operation, &t);
                if (r < 0) {
                        if (!exclusive && r == -EROFS)
                                log_debug_errno(r, "Failed to create shared lock for '%s', ignoring: %m", path);
                        else
                                return r;
                }
        }

        if (p) {
                make_lock_dir();

                r = make_lock_file(p, operation, global);
                if (r < 0) {
                        release_lock_file(&t);
                        return r;
                }
        } else
                *global = (LockFile) LOCK_FILE_INIT;

        *local = t;
        return 0;
}

int image_set_limit(Image *i, uint64_t referenced_max) {
        int r;

        assert(i);

        if (IMAGE_IS_VENDOR(i) || IMAGE_IS_HOST(i))
                return -EROFS;

        if (i->type != IMAGE_SUBVOLUME)
                return -EOPNOTSUPP;

        /* We set the quota both for the subvolume as well as for the
         * subtree. The latter is mostly for historical reasons, since
         * we didn't use to have a concept of subtree quota, and hence
         * only modified the subvolume quota. */

        (void) btrfs_qgroup_set_limit(i->path, 0, referenced_max);
        (void) btrfs_subvol_auto_qgroup(i->path, 0, true);
        r = btrfs_subvol_set_subtree_quota_limit(i->path, 0, referenced_max);
        if (r < 0)
                return r;

        (void) image_update_quota(i, -EBADF);
        return 0;
}

int image_read_metadata(Image *i) {
    _cleanup_(release_lock_file) LockFile global_lock = LOCK_FILE_INIT;
    _cleanup_(release_lock_file) LockFile local_lock = LOCK_FILE_INIT;
    int r;

    assert(i);

    r = image_path_lock(i->path, 0 /*LOCK_SH|LOCK_NB*/, &global_lock, &local_lock);
    if (r < 0)
        return r;

    switch (i->type) {
    case IMAGE_DIRECTORY:
    case IMAGE_RAW: {
        // Read hostname, machine-id, etc.
        // Replace Linux-specific loop device handling with just path checks
        char *hostname = NULL;
        char *machine_id_path = NULL;

        // Example: read /etc/hostname in image
        if (asprintf(&hostname, "%s/etc/hostname", i->path) > 0) {
            // open and read hostname file
            _cleanup_close_ int fd = -1;
            fd = open(hostname, O_RDONLY);
            if (fd >= 0) {
                char buf[256];
                ssize_t n = read(fd, buf, sizeof(buf)-1);
                if (n > 0) {
                    buf[n] = 0;
                    // store or log hostname
                }
            }
            free(hostname);
        }

        // Other metadata reading (machine-info, os-release, etc.) can follow similarly

        break;
    }

    case IMAGE_BLOCK: {
        // Only mark metadata valid, macOS cannot dissect partitions like Linux loop
        break;
    }

    default:
        return -EOPNOTSUPP;
    }

    i->metadata_valid = true;
    return 0;
}

int image_name_lock(const char *name, int operation, LockFile *ret) {
        const char *p;

        assert(name);
        assert(ret);

        /* Locks an image name, regardless of the precise path used. */

        if (streq(name, ".host"))
                return -EBUSY;

        if (!image_name_is_valid(name))
                return -EINVAL;

        if (getenv_bool("SYSTEMD_NSPAWN_LOCK") == 0) {
                *ret = (LockFile) LOCK_FILE_INIT;
                return 0;
        }

        make_lock_dir();

        p = strjoina("/run/systemd/nspawn/locks/name-", name);
        return make_lock_file(p, operation, ret);
}

bool image_in_search_path(
                ImageClass class,
                const char *root,
                const char *image) {

        const char *path;

        assert(image);

        NULSTR_FOREACH(path, image_search_path[class]) {
                const char *p, *q;
                size_t k;

                if (!empty_or_root(root)) {
                        q = path_startswith(path, root);
                        if (!q)
                                continue;
                } else
                        q = path;

                p = path_startswith(q, path);
                if (!p)
                        continue;

                /* Make sure there's a filename following */
                k = strcspn(p, "/");
                if (k == 0)
                        continue;

                p += k;

                /* Accept trailing slashes */
                if (p[strspn(p, "/")] == 0)
                        return true;
        }

        return false;
}

static const char* const image_type_table[_IMAGE_TYPE_MAX] = {
        [IMAGE_DIRECTORY] = "directory",
        [IMAGE_SUBVOLUME] = "subvolume",
        [IMAGE_RAW] = "raw",
        [IMAGE_BLOCK] = "block",
};

DEFINE_STRING_TABLE_LOOKUP(image_type, ImageType);
