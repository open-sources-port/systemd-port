#pragma once
/* compat/sys/capability.h
 *
 * Stub for macOS to allow Linux code using <sys/capability.h> to compile.
 * All functions are no-op or return dummy values.
 */

#ifdef __APPLE__

#include <stddef.h>

/* ------------------------------------------------------------------------- */
/* Types */

typedef int cap_value_t;   /* Linux uses an enum, use int */
typedef void* cap_t;       /* opaque pointer */

/* ------------------------------------------------------------------------- */
/* Capability retrieval / manipulation */

/* Get the capabilities of the current process */
static inline cap_t cap_get_proc(void) {
    return NULL;
}

/* Set the capabilities of the current process */
static inline int cap_set_proc(cap_t cap) {
    (void)cap;
    return 0; /* pretend success */
}

/* Free a capability object */
static inline int cap_free(void *cap) {
    (void)cap;
    return 0;
}

/* Clear all capabilities */
static inline int cap_clear(cap_t cap) {
    (void)cap;
    return 0;
}

/* Set capability flags */
static inline int cap_set_flag(cap_t cap, int flag, int ncap, const cap_value_t *caps, int value) {
    (void)cap; (void)flag; (void)ncap; (void)caps; (void)value;
    return 0;
}

/* Get capability flags (always returns 0) */
static inline int cap_get_flag(cap_t cap, cap_value_t cap_value, int flag, int *value) {
    (void)cap; (void)cap_value; (void)flag;
    if (value) *value = 0;
    return 0;
}

/* Merge capabilities (dummy) */
static inline int cap_merge(cap_t *dest, const cap_t *src) {
    (void)dest; (void)src;
    return 0;
}

/* Set the permitted, effective, inheritable sets (dummy) */
static inline int cap_set_proc_flag(cap_t cap, int flag) {
    (void)cap; (void)flag;
    return 0;
}

/* Return human-readable name of a capability (dummy) */
static inline const char* cap_to_text(cap_t cap, ssize_t *len) {
    if (len) *len = 0;
    (void)cap;
    return "";
}

/* Convert text to capability (dummy) */
static inline cap_t cap_from_text(const char *text) {
    (void)text;
    return NULL;
}

#endif /* __APPLE__ */
