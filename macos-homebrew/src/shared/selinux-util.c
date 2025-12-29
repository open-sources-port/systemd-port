/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include "selinux-util.h"

#include <sys_compat/errno.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Global SELinux state (always disabled on macOS)
 * ============================================================ */

bool mac_selinux_use(void) {
        return false;
}

void mac_selinux_retest(void) {
        /* no-op */
}

bool mac_selinux_enforcing(void) {
        return false;
}

int mac_selinux_init(void) {
        return 0;
}

void mac_selinux_maybe_reload(void) {
        /* no-op */
}

void mac_selinux_finish(void) {
        /* no-op */
}

/* ============================================================
 * Label operations (no-op)
 * ============================================================ */

int mac_selinux_fix_full(
                int atfd,
                const char *inode_path,
                const char *label_path,
                LabelFixFlags flags) {

        (void) atfd;
        (void) inode_path;
        (void) label_path;
        (void) flags;

        return 0;
}

int mac_selinux_apply(const char *path, const char *label) {
        (void) path;
        (void) label;
        return 0;
}

int mac_selinux_apply_fd(int fd, const char *path, const char *label) {
        (void) fd;
        (void) path;
        (void) label;
        return 0;
}

/* ============================================================
 * Label queries (always unsupported)
 * ============================================================ */

int mac_selinux_get_create_label_from_exe(const char *exe, char **label) {
        (void) exe;

        if (label)
                *label = NULL;

        return 0;
}

int mac_selinux_get_our_label(char **label) {
        if (label)
                *label = NULL;

        return 0;
}

int mac_selinux_get_child_mls_label(
                int socket_fd,
                const char *exe,
                const char *exec_label,
                char **label) {

        (void) socket_fd;
        (void) exe;
        (void) exec_label;

        if (label)
                *label = NULL;

        return 0;
}

char* mac_selinux_free(char *label) {
        free(label);
        return NULL;
}

/* ============================================================
 * File creation hooks (no-op)
 * ============================================================ */

int mac_selinux_create_file_prepare_at(int dirfd, const char *path, mode_t mode) {
        (void) dirfd;
        (void) path;
        (void) mode;
        return 0;
}

int mac_selinux_create_file_prepare_label(const char *path, const char *label) {
        (void) path;
        (void) label;
        return 0;
}

void mac_selinux_create_file_clear(void) {
        /* no-op */
}

/* ============================================================
 * Socket hooks (no-op)
 * ============================================================ */

int mac_selinux_create_socket_prepare(const char *label) {
        (void) label;
        return 0;
}

void mac_selinux_create_socket_clear(void) {
        /* no-op */
}

int mac_selinux_bind(int fd, const struct sockaddr *addr, socklen_t addrlen) {
        (void) fd;
        (void) addr;
        (void) addrlen;
        return 0;
}
