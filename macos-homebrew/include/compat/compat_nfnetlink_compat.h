// compat_nfnetlink_compat.h
#pragma once

#include <compat/compat_types.h>

#ifdef __linux__
    #include <linux/netfilter/nfnetlink_compat.h>
#else
    // macOS / BSD have no Netfilter. Provide minimal stubs.

    // Error codes or netlink message types (adjust as needed)
    #ifndef NFNL_MSG_COMPAT_GET
    #define NFNL_MSG_COMPAT_GET 0x10
    #endif

    #ifndef NFNL_MSG_COMPAT_NEW
    #define NFNL_MSG_COMPAT_NEW 0x11
    #endif

    #ifndef NFNL_MSG_COMPAT_DEL
    #define NFNL_MSG_COMPAT_DEL 0x12
    #endif

    // Stub struct – adjust if your code accesses fields
    struct nfgenmsg {
        uint8_t  nfgen_family;   // address family
        uint8_t  version;        // nfnetlink version
        uint16_t res_id;         // resource id
    };

    // Define compatibility macros if needed
    #ifndef NFNETLINK_V0
    #define NFNETLINK_V0 0
    #endif

    // Add other defines/structs as your code requires.
#endif
