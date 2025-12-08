#ifndef COMPAT_ARPHRD_TO_NAME_H
#define COMPAT_ARPHRD_TO_NAME_H

#ifdef __APPLE__

/*
 * Stub header for macOS/BSD builds.
 * Linux provides <linux/arphrd-to-name.h> for mapping ARPHRD_* to names.
 * This provides a minimal dummy implementation so code can compile.
 */

#include <net/if_arp.h>  /* for ARPHRD_* constants */

static inline const char *arphrd_to_name(int type) {
    switch (type) {
        case ARPHRD_ETHER:    return "ether";
#ifdef ARPHRD_LOOPBACK
        case ARPHRD_LOOPBACK: return "loopback";
#endif
#ifdef ARPHRD_INFINIBAND
        case ARPHRD_INFINIBAND: return "infiniband";
#endif
        default:              return "unknown";
    }
}

#endif

#endif /* COMPAT_ARPHRD_TO_NAME_H */
