/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <sys_compat/errno.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <linux/loop.h>

#include <linux/fs.h>
#include <basic/missing_mount.h>

#include "alloc-util.h"
#include "chase-symlinks.h"
#include "dissect-image.h"
#include "exec-util.h"
#include "extract-word.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "glyph-util.h"
#include "hashmap.h"
#include "label.h"
#include "libmount-util.h"
#include "missing_mount.h"
#include <sys_compat/missing_syscall.h>
#include "mkdir-label.h"
#include "mount-util.h"
#include "mountpoint-util.h"
#include "namespace-util.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "set.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "tmpfile-util.h"
#include "user-util.h"

#include <dirent.h>
#include <fts.h>

static int copy_file(const char *src, const char *dst) {
    int in = open(src, O_RDONLY);
    if (in < 0)
        return -errno;

    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) {
        close(in);
        return -errno;
    }

    char buf[8192];
    ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) {
        if (write(out, buf, n) != n) {
            close(in);
            close(out);
            return -errno;
        }
    }

    close(in);
    close(out);
    return (n < 0) ? -errno : 0;
}

static int copy_tree(const char *src, const char *dst) {
    FTS *fts;
    FTSENT *ent;
    char *paths[] = { (char *)src, NULL };
    int r = 0;

    fts = fts_open(paths, FTS_NOCHDIR | FTS_PHYSICAL, NULL);
    if (!fts)
        return -errno;

    while ((ent = fts_read(fts)) != NULL) {
        switch (ent->fts_info) {
        case FTS_D:
            // Directory: create corresponding path in dst
            {
                char path[PATH_MAX];
                snprintf(path, sizeof(path), "%s/%s", dst, ent->fts_name);
                if (mkdir(path, 0755) < 0 && errno != EEXIST) {
                    r = -errno;
                    goto finish;
                }
            }
            break;
        case FTS_F:
            {
                char path[PATH_MAX];
                snprintf(path, sizeof(path), "%s/%s", dst, ent->fts_name);
                r = copy_file(ent->fts_path, path);
                if (r < 0)
                    goto finish;
            }
            break;
        default:
            break;
        }
    }

finish:
    fts_close(fts);
    return r;
}

static int chmod_recursive(const char *path, mode_t mode) {
    struct stat st;
    if (lstat(path, &st) < 0)
        return -errno;

    if (chmod(path, mode) < 0)
        return -errno;

    if (!S_ISDIR(st.st_mode))
        return 0;

    DIR *d = opendir(path);
    if (!d)
        return -errno;

    struct dirent *de;
    int r = 0;
    while ((de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        char child[PATH_MAX];
        snprintf(child, sizeof(child), "%s/%s", path, de->d_name);
        r = chmod_recursive(child, mode);
        if (r < 0)
            break;
    }
    closedir(d);
    return r;
}

int mount_fd(const char *source,
             int target_fd,
             const char *filesystemtype,
             unsigned long mountflags,
             const void *data) {

        if (mount_mac(filesystemtype, target_fd, mountflags, (void *) data) < 0) {
                if (errno != ENOENT)
                        return -errno;

                /* ENOENT can mean two things: either that the source is missing, or that /proc/ isn't
                 * mounted. Check for the latter to generate better error messages. */
                if (proc_mounted() == 0)
                        return -ENOSYS;

                return -ENOENT;
        }

        return 0;
}

int mount_nofollow(
                const char *source,
                const char *target,
                const char *filesystemtype,
                unsigned long mountflags,
                const void *data) {

        _cleanup_close_ int fd = -1;

        /* In almost all cases we want to manipulate the mount table without following symlinks, hence
         * mount_nofollow() is usually the way to go. The only exceptions are environments where /proc/ is
         * not available yet, since we need /proc/self/fd/ for this logic to work. i.e. during the early
         * initialization of namespacing/container stuff where /proc is not yet mounted (and maybe even the
         * fs to mount) we can only use traditional mount() directly.
         *
         * Note that this disables following only for the final component of the target, i.e symlinks within
         * the path of the target are honoured, as are symlinks in the source path everywhere. */

        fd = open(target, O_PATH|O_CLOEXEC|O_NOFOLLOW);
        if (fd < 0)
                return -errno;

        return mount_fd(source, fd, filesystemtype, mountflags, data);
}

/* Recursively unmount all mounts under 'prefix' on macOS */
int umount_recursive(const char *prefix, int flags) {
    int n = 0;
    bool again;

    do {
        struct statfs *mntbuf;
        int count = getmntinfo(&mntbuf, MNT_NOWAIT);
        if (!count) {
            perror("getmntinfo");
            return -1;
        }

        again = false;

        for (int i = 0; i < count; i++) {
            const char *path = mntbuf[i].f_mntonname; // mount point
            const char *fstype = mntbuf[i].f_fstypename;

            // Skip Linux-only filesystems (optional)
            if (strcmp(fstype, "proc") == 0 ||
                strcmp(fstype, "sysfs") == 0 ||
                strcmp(fstype, "tmpfs") == 0)
            {
                continue;
            }

            if (!path_startswith(path, prefix))
                continue;

            // macOS unmount
            if (unmount(path, flags) < 0) {
                fprintf(stderr, "Failed to unmount %s: %s\n", path, strerror(errno));
                continue;
            }

            printf("Successfully unmounted %s\n", path);
            n++;
            again = true;

            break; // restart iteration to handle nested mounts
        }

    } while (again);

    return n;
}

#define MS_CONVERTIBLE_FLAGS (MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|MS_NOSYMFOLLOW)

// static uint64_t ms_flags_to_mount_attr(unsigned long a) {
//         uint64_t f = 0;

//         if (FLAGS_SET(a, MS_RDONLY))
//                 f |= MOUNT_ATTR_RDONLY;

//         if (FLAGS_SET(a, MS_NOSUID))
//                 f |= MOUNT_ATTR_NOSUID;

//         if (FLAGS_SET(a, MS_NODEV))
//                 f |= MOUNT_ATTR_NODEV;

//         if (FLAGS_SET(a, MS_NOEXEC))
//                 f |= MOUNT_ATTR_NOEXEC;

//         if (FLAGS_SET(a, MS_NOSYMFOLLOW))
//                 f |= MOUNT_ATTR_NOSYMFOLLOW;

//         return f;
// }

// static bool skip_mount_set_attr = false;

/* Use this function only if you do not have direct access to /proc/self/mountinfo but the caller can open it
 * for you. This is the case when /proc is masked or not mounted. Otherwise, use bind_remount_recursive. */
int bind_remount_recursive_with_mountinfo(
    const char *prefix,
    unsigned long new_flags,
    unsigned long flags_mask,
    char **deny_list,
    FILE *proc_self_mountinfo) {

    (void)flags_mask;
    (void)deny_list;
    (void)proc_self_mountinfo;

    // macOS simplified implementation
    struct statfs *mntbuf;
    int n_mnt, i;

    n_mnt = getmntinfo(&mntbuf, MNT_NOWAIT);
    if (n_mnt == 0)
        return -errno;

    for (i = 0; i < n_mnt; i++) {
        const char *path = mntbuf[i].f_mntonname;
        if (strncmp(path, prefix, strlen(prefix)) != 0)
            continue;

        int flags = mntbuf[i].f_flags;
        int new_mount_flags = (new_flags & 1) ? MNT_RDONLY : 0;

        if ((flags & MNT_RDONLY) != new_mount_flags) {
            if (mount(mntbuf[i].f_fstypename, path, flags | MNT_UPDATE | new_mount_flags, NULL) < 0) {
                if (errno == EBUSY || errno == EPERM) {
                    fprintf(stderr, "Warning: Failed to remount %s: %s\n", path, strerror(errno));
                    continue;
                }
                return -errno;
            }
        }
    }

    return 0;
}

int bind_remount_one_with_mountinfo(
    const char *path,
    unsigned long new_flags,
    unsigned long flags_mask,
    FILE *proc_self_mountinfo)  // keep this param to match original signature
{
    (void)flags_mask;
    (void)proc_self_mountinfo;

    assert(path);

    struct statfs *mntbuf;
    int n_mnt, i;

    // Get list of mounted file systems
    n_mnt = getmntinfo(&mntbuf, MNT_NOWAIT);
    if (n_mnt == 0)
        return -errno;

    // Find the mount point
    for (i = 0; i < n_mnt; i++) {
        if (strcmp(mntbuf[i].f_mntonname, path) == 0) {
            int flags = mntbuf[i].f_flags;
            int new_mount_flags = (new_flags & 1) ? MNT_RDONLY : 0;

            if ((flags & MNT_RDONLY) != new_mount_flags) {
                if (mount(mntbuf[i].f_fstypename, path, flags | MNT_UPDATE | new_mount_flags, NULL) < 0) {
                    if ((flags ^ new_flags) & 1)  // Only fail if requested flags differ
                        return -errno;
                    fprintf(stderr, "Warning: Failed to remount '%s', ignoring since flags already match.\n", path);
                }
            }
            return 0;  // Successfully processed
        }
    }

    // Mount point not found
    if (access(path, F_OK) < 0)
        return -errno;

    return -EINVAL; // path exists but not a recognized mount point
}

int mount_move_root(const char *path) {
        assert(path);

        if (chdir(path) < 0)
                return -errno;

        // if (mount(path, "/", NULL, MS_MOVE, NULL) < 0)
        //         return -errno;

        if (chroot(".") < 0)
                return -errno;

        return RET_NERRNO(chdir("/"));
}

int repeat_unmount(const char *path, int flags) {
    bool done = false;

    assert(path);

    for (;;) {
        if (unmount(path, flags) < 0) {
            if (errno == EINVAL) // Not a mount point
                return done ? 0 : -EINVAL;
            if (errno == ENOENT) // Already gone
                return done ? 0 : -ENOENT;
            return -errno;
        }

        done = true;
    }
}

int mode_to_inaccessible_node(
                const char *runtime_dir,
                mode_t mode,
                char **ret) {

        /* This function maps a node type to a corresponding inaccessible file node. These nodes are created
         * during early boot by PID 1. In some cases we lacked the privs to create the character and block
         * devices (maybe because we run in an userns environment, or miss CAP_SYS_MKNOD, or run with a
         * devices policy that excludes device nodes with major and minor of 0), but that's fine, in that
         * case we use an AF_UNIX file node instead, which is not the same, but close enough for most
         * uses. And most importantly, the kernel allows bind mounts from socket nodes to any non-directory
         * file nodes, and that's the most important thing that matters.
         *
         * Note that the runtime directory argument shall be the top-level runtime directory, i.e. /run/ if
         * we operate in system context and $XDG_RUNTIME_DIR if we operate in user context. */

        _cleanup_free_ char *d = NULL;
        const char *node = NULL;

        assert(ret);

        if (!runtime_dir)
                runtime_dir = "/run";

        switch (mode & S_IFMT) {
                case S_IFREG:
                        node = "/systemd/inaccessible/reg";
                        break;

                case S_IFDIR:
                        node = "/systemd/inaccessible/dir";
                        break;

                case S_IFCHR:
                        node = "/systemd/inaccessible/chr";
                        break;

                case S_IFBLK:
                        node = "/systemd/inaccessible/blk";
                        break;

                case S_IFIFO:
                        node = "/systemd/inaccessible/fifo";
                        break;

                case S_IFSOCK:
                        node = "/systemd/inaccessible/sock";
                        break;
        }
        if (!node)
                return -EINVAL;

        d = path_join(runtime_dir, node);
        if (!d)
                return -ENOMEM;

        /* On new kernels unprivileged users are permitted to create 0:0 char device nodes (because they also
         * act as whiteout inode for overlayfs), but no other char or block device nodes. On old kernels no
         * device node whatsoever may be created by unprivileged processes. Hence, if the caller asks for the
         * inaccessible block device node let's see if the block device node actually exists, and if not,
         * fall back to the character device node. From there fall back to the socket device node. This means
         * in the best case we'll get the right device node type — but if not we'll hopefully at least get a
         * device node at all. */

        if (S_ISBLK(mode) &&
            access(d, F_OK) < 0 && errno == ENOENT) {
                free(d);
                d = path_join(runtime_dir, "/systemd/inaccessible/chr");
                if (!d)
                        return -ENOMEM;
        }

        if (IN_SET(mode & S_IFMT, S_IFBLK, S_IFCHR) &&
            access(d, F_OK) < 0 && errno == ENOENT) {
                free(d);
                d = path_join(runtime_dir, "/systemd/inaccessible/sock");
                if (!d)
                        return -ENOMEM;
        }

        *ret = TAKE_PTR(d);
        return 0;
}

int mount_flags_to_string(unsigned long flags, char **ret) {
        static const struct {
                unsigned long flag;
                const char *name;
        } map[] = {
                { .flag = MS_RDONLY,      .name = "MS_RDONLY",      },
                { .flag = MS_NOSUID,      .name = "MS_NOSUID",      },
                { .flag = MS_NODEV,       .name = "MS_NODEV",       },
                { .flag = MS_NOEXEC,      .name = "MS_NOEXEC",      },
                // { .flag = MS_SYNCHRONOUS, .name = "MS_SYNCHRONOUS", },
                // { .flag = MS_REMOUNT,     .name = "MS_REMOUNT",     },
                // { .flag = MS_MANDLOCK,    .name = "MS_MANDLOCK",    },
                // { .flag = MS_DIRSYNC,     .name = "MS_DIRSYNC",     },
                { .flag = MS_NOSYMFOLLOW, .name = "MS_NOSYMFOLLOW", },
                // { .flag = MS_NOATIME,     .name = "MS_NOATIME",     },
                // { .flag = MS_NODIRATIME,  .name = "MS_NODIRATIME",  },
                { .flag = MS_BIND,        .name = "MS_BIND",        },
                { .flag = MS_MOVE,        .name = "MS_MOVE",        },
                { .flag = MS_REC,         .name = "MS_REC",         },
                // { .flag = MS_SILENT,      .name = "MS_SILENT",      },
                // { .flag = MS_POSIXACL,    .name = "MS_POSIXACL",    },
                // { .flag = MS_UNBINDABLE,  .name = "MS_UNBINDABLE",  },
                { .flag = MS_PRIVATE,     .name = "MS_PRIVATE",     },
                { .flag = MS_SLAVE,       .name = "MS_SLAVE",       },
                { .flag = MS_SHARED,      .name = "MS_SHARED",      },
                { .flag = MS_RELATIME,    .name = "MS_RELATIME",    },
                { .flag = MS_KERNMOUNT,   .name = "MS_KERNMOUNT",   },
                { .flag = MS_I_VERSION,   .name = "MS_I_VERSION",   },
                { .flag = MS_STRICTATIME, .name = "MS_STRICTATIME", },
                { .flag = MS_LAZYTIME,    .name = "MS_LAZYTIME",    },
        };
        _cleanup_free_ char *str = NULL;

        assert(ret);

        for (size_t i = 0; i < ELEMENTSOF(map); i++)
                if (flags & map[i].flag) {
                        if (!strextend_with_separator(&str, "|", map[i].name))
                                return -ENOMEM;
                        flags &= ~map[i].flag;
                }

        if (!str || flags != 0)
                if (strextendf_with_separator(&str, "|", "%lx", flags) < 0)
                        return -ENOMEM;

        *ret = TAKE_PTR(str);
        return 0;
}

int mount_verbose_full(
                int error_log_level,
                const char *what,
                const char *where,
                const char *type,
                unsigned long flags,
                const char *options,
                bool follow_symlink) {

        _cleanup_free_ char *fl = NULL, *o = NULL;
        unsigned long f;
        int r;

        r = mount_option_mangle(options, flags, &f, &o);
        if (r < 0)
                return log_full_errno(error_log_level, r,
                                      "Failed to mangle mount options %s: %m",
                                      strempty(options));

        (void) mount_flags_to_string(f, &fl);

        if (!what && !type)
                log_debug("Mounting %s (%s \"%s\")...",
                          where, strnull(fl), strempty(o));
        else if ((f & MS_BIND) && !type)
                log_debug("Bind-mounting %s on %s (%s \"%s\")...",
                          what, where, strnull(fl), strempty(o));
        else if (f & MS_MOVE)
                log_debug("Moving mount %s %s %s (%s \"%s\")...",
                          what, special_glyph(SPECIAL_GLYPH_ARROW_RIGHT), where, strnull(fl), strempty(o));
        else
                log_debug("Mounting %s (%s) on %s (%s \"%s\")...",
                          strna(what), strna(type), where, strnull(fl), strempty(o));

        #ifdef __APPLE__
                // macOS mount: int mount(const char *type, const char *dir, int flags, void *data);
                // 'what' (source) is not separate; encode it in options if needed
                r = mount(type, where, f, (void *)o);
        #else
                if (follow_symlink)
                        r = RET_NERRNO(mount(what, where, type, f, o));
                else
                        r = mount_nofollow(what, where, type, f, o);
        #endif

        if (r < 0)
                return log_full_errno(error_log_level, r,
                                      "Failed to mount %s (type %s) on %s (%s \"%s\"): %m",
                                      strna(what), strna(type), where, strnull(fl), strempty(o));
        return 0;
}


int umount_verbose(
        int error_log_level,
        const char *what,
        int flags) {

        assert(what);

        log_debug("Umounting %s...", what);

        if (unmount(what, 0) < 0)
                return log_full_errno(error_log_level, errno,
                                        "Failed to unmount %s: %m", what);

        return 0;
}

int mount_option_mangle(
        const char *options,
        unsigned long mount_flags,
        unsigned long *ret_mount_flags,
        char **ret_remaining_options) {

    _cleanup_free_ char *ret = NULL;

    assert(ret_mount_flags);
    assert(ret_remaining_options);

    // On macOS, mount flags are mostly passed as-is.
    // Just return the original flags and the options string.
    *ret_mount_flags = mount_flags;

    if (options && *options)
        *ret_remaining_options = strdup(options);  // copy options string
    else
        *ret_remaining_options = NULL;

    return 0;
}

static int mount_in_namespace(
        pid_t target __attribute__((unused)),
        const char *propagate_path,
        const char *incoming_path,
        const char *src,
        const char *dest,
        bool read_only,
        bool make_file_or_directory,
        const MountOptions *options __attribute__((unused)),
        bool is_image) {

    _cleanup_free_ char *chased_src_path = NULL;
    struct stat st;
    int r;

    assert(propagate_path);
    assert(incoming_path);
    assert(src);
    assert(dest);

    // Resolve symlinks
//     r = realpath(src, &chased_src_path);
    chased_src_path = realpath(src, NULL);
    if (!chased_src_path) {
        return log_debug_errno(errno, "Failed to resolve source path %s: %m", src);
    }

    if (stat(chased_src_path, &st) < 0)
        return log_debug_errno(errno, "Failed to stat resolved source path %s: %m", chased_src_path);

    if (make_file_or_directory) {
        if (S_ISDIR(st.st_mode)) {
            if (mkdir(dest, 0755) < 0 && errno != EEXIST)
                return log_debug_errno(errno, "Failed to create directory %s: %m", dest);
        } else {
            int fd = open(dest, O_CREAT | O_EXCL | O_WRONLY, 0644);
            if (fd < 0 && errno != EEXIST)
                return log_debug_errno(errno, "Failed to create file %s: %m", dest);
            if (fd >= 0)
                close(fd);
        }
    }

        // macOS doesn't have MS_BIND, emulate by copying or using symlink
        if (is_image || S_ISDIR(st.st_mode)) {
                // Create temporary directory
                char tmp_dest[PATH_MAX];
                snprintf(tmp_dest, sizeof(tmp_dest), "/tmp/mnt.XXXXXX");
                if (!mkdtemp(tmp_dest))
                        return log_debug_errno(errno, "Failed to create temp dir %s: %m", tmp_dest);

                // Copy directory contents (recursive)
                r = copy_tree(chased_src_path, tmp_dest); // Implement copy_tree() to recursively copy files
                if (r < 0)
                        return log_debug_errno(r, "Failed to copy %s -> %s", chased_src_path, tmp_dest);

                // Optionally enforce read-only
                if (read_only)
                        r = chmod_recursive(tmp_dest, 0555); // Implement chmod_recursive() to apply permissions
                        if (r < 0)
                        return log_debug_errno(r, "Failed to set read-only for %s", tmp_dest);

                // Move temporary dir to final destination
                if (rename(tmp_dest, dest) < 0)
                        return log_debug_errno(errno, "Failed to move %s -> %s: %m", tmp_dest, dest);

        } else {
                // For single files, use symlink
                if (symlink(chased_src_path, dest) < 0 && errno != EEXIST)
                        return log_debug_errno(errno, "Failed to symlink %s -> %s: %m", chased_src_path, dest);
        }

    return 0;
}

int bind_mount_in_namespace(
                pid_t target,
                const char *propagate_path,
                const char *incoming_path,
                const char *src,
                const char *dest,
                bool read_only,
                bool make_file_or_directory) {

        return mount_in_namespace(target, propagate_path, incoming_path, src, dest, read_only, make_file_or_directory, NULL, false);
}

int mount_image_in_namespace(
                pid_t target,
                const char *propagate_path,
                const char *incoming_path,
                const char *src,
                const char *dest,
                bool read_only,
                bool make_file_or_directory,
                const MountOptions *options) {

        return mount_in_namespace(target, propagate_path, incoming_path, src, dest, read_only, make_file_or_directory, options, true);
}

int make_mount_point(const char *path) {
        int r;

        assert(path);

        /* If 'path' is already a mount point, does nothing and returns 0. If it is not it makes it one, and returns 1. */

        r = path_is_mount_point(path, NULL, 0);
        if (r < 0)
                return log_debug_errno(r, "Failed to determine whether '%s' is a mount point: %m", path);
        if (r > 0)
                return 0;

        r = mount_nofollow_verbose(LOG_DEBUG, path, path, NULL, MS_BIND|MS_REC, NULL);
        if (r < 0)
                return r;

        return 1;
}

// static int make_userns(uid_t uid_shift, uid_t uid_range, uid_t owner, RemountIdmapping idmapping) {
//         _cleanup_close_ int userns_fd = -1;
//         _cleanup_free_ char *line = NULL;

//         /* Allocates a userns file descriptor with the mapping we need. For this we'll fork off a child
//          * process whose only purpose is to give us a new user namespace. It's killed when we got it. */

//         if (IN_SET(idmapping, REMOUNT_IDMAPPING_NONE, REMOUNT_IDMAPPING_HOST_ROOT)) {
//                 if (asprintf(&line, UID_FMT " " UID_FMT " " UID_FMT "\n", 0u, uid_shift, uid_range) < 0)
//                         return log_oom_debug();

//                 /* If requested we'll include an entry in the mapping so that the host root user can make
//                  * changes to the uidmapped mount like it normally would. Specifically, we'll map the user
//                  * with UID_MAPPED_ROOT on the backing fs to UID 0. This is useful, since nspawn code wants
//                  * to create various missing inodes in the OS tree before booting into it, and this becomes
//                  * very easy and straightforward to do if it can just do it under its own regular UID. Note
//                  * that in that case the container's runtime uidmap (i.e. the one the container payload
//                  * processes run in) will leave this UID unmapped, i.e. if we accidentally leave files owned
//                  * by host root in the already uidmapped tree around they'll show up as owned by 'nobody',
//                  * which is safe. (Of course, we shouldn't leave such inodes around, but always chown() them
//                  * to the container's own UID range, but it's good to have a safety net, in case we
//                  * forget it.) */
//                 if (idmapping == REMOUNT_IDMAPPING_HOST_ROOT)
//                         if (strextendf(&line,
//                                        UID_FMT " " UID_FMT " " UID_FMT "\n",
//                                        UID_MAPPED_ROOT, 0u, 1u) < 0)
//                                 return log_oom_debug();
//         }

//         if (idmapping == REMOUNT_IDMAPPING_HOST_OWNER) {
//                 /* Remap the owner of the bind mounted directory to the root user within the container. This
//                  * way every file written by root within the container to the bind-mounted directory will
//                  * be owned by the original user. All other user will remain unmapped. */
//                 if (asprintf(&line, UID_FMT " " UID_FMT " " UID_FMT "\n", owner, uid_shift, 1u) < 0)
//                         return log_oom_debug();
//         }

//         /* We always assign the same UID and GID ranges */
//         userns_fd = userns_acquire(line, line);
//         if (userns_fd < 0)
//                 return log_debug_errno(userns_fd, "Failed to acquire new userns: %m");

//         return TAKE_FD(userns_fd);
// }

int remount_idmap(
        const char *p,
        uid_t uid_shift __attribute__((unused)),
        uid_t uid_range __attribute__((unused)),
        uid_t owner,
        RemountIdmapping idmapping __attribute__((unused))) {

    assert(p);

    // macOS cannot remap UID/GID on a mount
    // We can optionally set ownership of files if 'owner' is specified
    if (owner != (uid_t) -1) {
        int r = chown(p, owner, -1);
        if (r < 0)
            return log_debug_errno(errno, "Failed to change ownership of %s: %m", p);
    }

    // Return success, but no UID mapping applied
    return 0;
}

int make_mount_point_inode_from_stat(const struct stat *st, const char *dest, mode_t mode) {
        assert(st);
        assert(dest);

        if (S_ISDIR(st->st_mode))
                return mkdir_label(dest, mode);
        else
                return RET_NERRNO(mknod(dest, S_IFREG|(mode & ~0111), 0));
}

int make_mount_point_inode_from_path(const char *source, const char *dest, mode_t mode) {
        struct stat st;

        assert(source);
        assert(dest);

        if (stat(source, &st) < 0)
                return -errno;

        return make_mount_point_inode_from_stat(&st, dest, mode);
}
