#ifndef COMPAT_ARPHRD_FROM_NAME_H
#define COMPAT_ARPHRD_FROM_NAME_H

#ifdef __APPLE__

/*
 * Stub header for macOS/BSD builds.
 * Linux provides <linux/arphrd-from-name.h> to map ARPHRD_* to names.
 * Here we provide a minimal dummy implementation.
 */

#include <net/if_arp.h>   /* for ARPHRD_* constants if needed */

static inline int arphrd_from_name(const char *name) {
    /* Always fail on macOS – no mapping available */
    (void)name;
    return -1;
}

static inline const char *arphrd_to_name(int type) {
    (void)type;
    return "unknown";
}

#endif

#endif /* COMPAT_ARPHRD_FROM_NAME_H */
