/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <stdio.h>

/* This needs to be after sys/mount.h */
// #include <libmount.h>

#include <basic/macro.h>
#include <sys/mount.h>
#include <stddef.h>
#include <stdint.h>

// DEFINE_TRIVIAL_CLEANUP_FUNC_FULL(struct libmnt_table*, mnt_free_table, NULL);
// DEFINE_TRIVIAL_CLEANUP_FUNC_FULL(struct libmnt_iter*, mnt_free_iter, NULL);

// static inline int libmount_parse(
//                 const char *path,
//                 FILE *source,
//                 struct libmnt_table **ret_table,
//                 struct libmnt_iter **ret_iter) {

//         _cleanup_(mnt_free_tablep) struct libmnt_table *table = NULL;
//         _cleanup_(mnt_free_iterp) struct libmnt_iter *iter = NULL;
//         int r;

//         /* Older libmount seems to require this. */
//         assert(!source || path);

//         table = mnt_new_table();
//         iter = mnt_new_iter(MNT_ITER_FORWARD);
//         if (!table || !iter)
//                 return -ENOMEM;

//         /* If source or path are specified, we use on the functions which ignore utab.
//          * Only if both are empty, we use mnt_table_parse_mtab(). */

//         if (source)
//                 r = mnt_table_parse_stream(table, source, path);
//         else if (path)
//                 r = mnt_table_parse_file(table, path);
//         else
//                 r = mnt_table_parse_mtab(table, NULL);
//         if (r < 0)
//                 return r;

//         *ret_table = TAKE_PTR(table);
//         *ret_iter = TAKE_PTR(iter);
//         return 0;
// }

// Dummy type definitions
typedef struct {
    // Add fields if needed for macOS-specific mount handling
} mnt_context;

typedef struct {
    // Placeholder for mount entries
} mount_entry;

// Dummy function stubs
static inline int mnt_context_new(mnt_context **ctx) {
    *ctx = NULL;  // nothing to allocate
    return 0;     // success
}

static inline void mnt_context_free(mnt_context *ctx) {
    // nothing to free
}

static inline int mnt_context_scan(mnt_context *ctx) {
    return 0; // nothing to scan
}

static inline mount_entry *mnt_context_next(mnt_context *ctx) {
    return NULL; // no entries
}

static inline const char *mount_entry_get_source(mount_entry *entry) {
    return NULL;
}

static inline const char *mount_entry_get_target(mount_entry *entry) {
    return NULL;
}

static inline const char *mount_entry_get_fstype(mount_entry *entry) {
    return NULL;
}

// Dummy types for macOS
typedef struct libmnt_table libmnt_table;
typedef struct libmnt_iter libmnt_iter;

static inline int libmount_parse(
    const char *path,
    FILE *source,
    libmnt_table **ret_table,
    libmnt_iter **ret_iter
) {
    if (ret_table)
        *ret_table = NULL;
    if (ret_iter)
        *ret_iter = NULL;
    (void)path;    // suppress unused parameter warning
    (void)source;  // suppress unused parameter warning
    return 0;      // success
}
