/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include "socket-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ucred.h>
#include <stddef.h>

int fd_set_rcvbuf(int fd, size_t n, bool increase) {
    int cur = 0;
    socklen_t len = sizeof(cur);
    int newv;

    if (fd < 0)
        return -EINVAL;

    if (n > INT_MAX)
        n = INT_MAX;

    /*
     * If we only want to increase, read current value first
     */
    if (increase) {
        if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &cur, &len) < 0)
            return -errno;

        if ((size_t) cur >= n)
            return 0;
    }

    newv = (int) n;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &newv, sizeof(newv)) < 0)
        return -errno;

    return 0;
}

int fd_set_sndbuf(int fd, size_t n, bool increase) {
    int cur = 0;
    socklen_t len = sizeof(cur);
    int newv;

    if (fd < 0)
        return -EINVAL;

    if (n > INT_MAX)
        n = INT_MAX;

    if (increase) {
        if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &cur, &len) < 0)
            return -errno;

        if ((size_t) cur >= n)
            return 0;
    }

    newv = (int) n;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &newv, sizeof(newv)) < 0)
        return -errno;

    return 0;
}

struct cmsghdr* cmsg_find(struct msghdr *mh, int level, int type, socklen_t min_len) {
    struct cmsghdr *cmsg;

    if (!mh)
        return NULL;

    for (cmsg = CMSG_FIRSTHDR(mh);
         cmsg != NULL;
         cmsg = CMSG_NXTHDR(mh, cmsg)) {

        if (cmsg->cmsg_level != level)
            continue;

        if (cmsg->cmsg_type != type)
            continue;

        if (cmsg->cmsg_len < min_len)
            continue;

        return cmsg;
    }

    return NULL;
}

int connect_unix_path(int fd, int dir_fd, const char *path) {
    struct sockaddr_un sa;

    if (!path)
        return -EINVAL;

    if (dir_fd != AT_FDCWD)
        return -ENOTSUP; /* macOS: no dirfd-based AF_UNIX */

    if (strlen(path) >= sizeof(sa.sun_path))
        return -ENAMETOOLONG;

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
        return -errno;

    return 0;
}

/* ------------------- Socket Type Table ------------------- */
static const char * const socket_address_type_table[] = {
    [SOCK_STREAM] = "Stream",
    [SOCK_DGRAM]  = "Datagram",
    [SOCK_RAW]    = "Raw",
};

const char* socket_address_type_to_string(int t) {
    if (t < 0 || t >= (int)(sizeof(socket_address_type_table)/sizeof(*socket_address_type_table)))
        return "unknown";
    return socket_address_type_table[t];
}

int socket_address_type_from_string(const char *s) {
    if (!s) return -1;
    for (int i = 0; i < (int)(sizeof(socket_address_type_table)/sizeof(*socket_address_type_table)); i++)
        if (socket_address_type_table[i] && strcmp(socket_address_type_table[i], s) == 0)
            return i;
    return -1;
}

/* ------------------- Socket Address Helpers ------------------- */
bool socket_address_equal(const SocketAddress *a, const SocketAddress *b) {
    if (!a || !b) return false;
    if (a->sockaddr.sa.sa_family != b->sockaddr.sa.sa_family) return false;

    switch (a->sockaddr.sa.sa_family) {
        case AF_INET:
            return a->sockaddr.in.sin_port == b->sockaddr.in.sin_port &&
                   a->sockaddr.in.sin_addr.s_addr == b->sockaddr.in.sin_addr.s_addr;
        case AF_INET6:
            return a->sockaddr.in6.sin6_port == b->sockaddr.in6.sin6_port &&
                   memcmp(&a->sockaddr.in6.sin6_addr, &b->sockaddr.in6.sin6_addr, sizeof(struct in6_addr)) == 0;
        case AF_UNIX:
            return strncmp(a->sockaddr.un.sun_path, b->sockaddr.un.sun_path, sizeof(a->sockaddr.un.sun_path)) == 0;
        default:
            return false;
    }
}

const char* socket_address_get_path(const SocketAddress *a) {
    if (!a) return NULL;
    if (a->sockaddr.sa.sa_family != AF_UNIX) return NULL;
    return a->sockaddr.un.sun_path;
}

int sockaddr_un_unlink(const struct sockaddr_un *sa) {
    if (!sa || sa->sun_path[0] == 0) return 0;
    return unlink(sa->sun_path);
}

/* ------------------- Listening / Binding ------------------- */
int socket_address_listen(
        const SocketAddress *a,
        int flags,
        int backlog,
        SocketAddressBindIPv6Only only,
        const char *bind_to_device,
        bool reuse_port,
        bool free_bind,
        bool transparent,
        mode_t directory_mode,
        mode_t socket_mode,
        const char *label) {

    int fd = socket(a->sockaddr.sa.sa_family, a->type, a->protocol);
    if (fd < 0) return -errno;

    if (reuse_port) setsockopt_int(fd, SOL_SOCKET, SO_REUSEADDR, 1);

    if (a->sockaddr.sa.sa_family == AF_UNIX) {
        char path_copy[sizeof(a->sockaddr.un.sun_path)];

        strncpy(path_copy, a->sockaddr.un.sun_path, sizeof(path_copy));
        path_copy[sizeof(path_copy) - 1] = '\0';

        mkdir(dirname(path_copy), directory_mode);

        // mkdir(dirname(a->sockaddr.un.sun_path), directory_mode);
        //sockaddr_un_set_path(&((struct sockaddr_un*) &a->sockaddr)->sun_path, a->sockaddr.un.sun_path);
        struct sockaddr_un *un = (struct sockaddr_un *) &a->sockaddr;
        sockaddr_un_set_path(un, un->sun_path);

        unlink(a->sockaddr.un.sun_path);
    }

    if (bind(fd, &a->sockaddr.sa, a->size) < 0) {
        close(fd);
        return -errno;
    }

    if (listen(fd, backlog) < 0) {
        close(fd);
        return -errno;
    }

    return fd;
}

/* ------------------- Peer Credentials ------------------- */
int getpeercred(int fd, struct ucred *ucred) {
    socklen_t sz = sizeof(*ucred);
    if (getsockopt(fd, SOL_LOCAL, LOCAL_PEERCRED, ucred, &sz) < 0) return -errno;
    return 0;
}

/* ------------------- IP TOS table ------------------- */
static const char * const ip_tos_table[] = {
    [0] = "default",
    [1] = "low-delay",
};

int ip_tos_from_string(const char *s) {
    if (!s) return -1;
    for (int i = 0; i < (int)(sizeof(ip_tos_table)/sizeof(*ip_tos_table)); i++)
        if (ip_tos_table[i] && strcmp(ip_tos_table[i], s) == 0)
            return i;
    return -1;
}

int ip_tos_to_string_alloc(int i, char **s) {
    if (!s) return -EINVAL;
    if (i < 0 || i >= (int)(sizeof(ip_tos_table)/sizeof(*ip_tos_table))) {
        *s = strdup("unknown");
        return *s ? 0 : -ENOMEM;
    }
    *s = strdup(ip_tos_table[i]);
    return *s ? 0 : -ENOMEM;
}

/* ------------------- IPv6 helpers ------------------- */
bool socket_ipv6_is_supported(void) {
    int fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd < 0) return false;
    close(fd);
    return true;
}

bool socket_ipv6_is_enabled(void) {
    /* macOS always enables IPv6 if supported */
    return socket_ipv6_is_supported();
}

/* ------------------- Pretty Print ------------------- */
int sockaddr_pretty(const struct sockaddr *_sa, socklen_t salen, bool translate_ipv6, bool include_port, char **ret) {
    char host[INET6_ADDRSTRLEN] = {0};
    uint16_t port = 0;

    switch (_sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &((struct sockaddr_in*)_sa)->sin_addr, host, sizeof(host));
            port = ntohs(((struct sockaddr_in*)_sa)->sin_port);
            break;
        case AF_INET6:
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)_sa)->sin6_addr, host, sizeof(host));
            port = ntohs(((struct sockaddr_in6*)_sa)->sin6_port);
            break;
        case AF_UNIX:
            snprintf(host, sizeof(host), "%s", ((struct sockaddr_un*)_sa)->sun_path);
            break;
        default:
            snprintf(host, sizeof(host), "unknown");
            break;
    }

    if (include_port && _sa->sa_family != AF_UNIX) {
        *ret = malloc(strlen(host) + 16);
        if (!*ret) return -ENOMEM;
        snprintf(*ret, strlen(host) + 16, "%s:%u", host, port);
    } else {
        *ret = strdup(host);
    }

    return *ret ? 0 : -ENOMEM;
}

/* getpeername pretty */
int getpeername_pretty(int fd, bool include_port, char **ret) {
    struct sockaddr_storage sa;
    socklen_t sl = sizeof(sa);
    if (getpeername(fd, (struct sockaddr*)&sa, &sl) < 0) return -errno;
    return sockaddr_pretty((struct sockaddr*)&sa, sl, true, include_port, ret);
}

/* getsockname pretty */
int getsockname_pretty(int fd, char **ret) {
    struct sockaddr_storage sa;
    socklen_t sl = sizeof(sa);
    if (getsockname(fd, (struct sockaddr*)&sa, &sl) < 0) return -errno;
    return sockaddr_pretty((struct sockaddr*)&sa, sl, true, true, ret);
}

/* ------------------- Unix Path helpers ------------------- */
int sockaddr_un_set_path(struct sockaddr_un *ret, const char *path) {
    size_t len = strnlen(path, sizeof(ret->sun_path) - 1);
    strncpy(ret->sun_path, path, len);
    ret->sun_path[len] = '\0';
    return 0;
}
