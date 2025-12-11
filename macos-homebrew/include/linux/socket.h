// compat/linux_socket.h (for macOS)
#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/ucred.h>
#include <fcntl.h>

// Linux -> macOS mappings

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC FD_CLOEXEC
#endif

// Linux AF_ numbers (optional)
#define AF_NETLINK  16     // stub, not functional
#define AF_PACKET   17     // stub, not functional

// Linux SO_ options (make them no-op if needed)
#ifndef SO_PEERCRED
#define SO_PEERCRED LOCAL_PEERCRED
#endif

// Missing features: BINDTODEVICE, PACKET sockets, NETLINK
static inline int setsockopt_noop(int fd, int level, int opt, const void *v, socklen_t l) {
    return 0; // always succeed
}

#ifndef SO_BINDTODEVICE
#define SO_BINDTODEVICE 25   // dummy
#endif
