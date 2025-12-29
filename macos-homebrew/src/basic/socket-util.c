/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include "socket-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys_compat/errno.h>
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
#include <ctype.h>
#include <net/if.h> // for IFNAMSIZ
#include <linux/in.h>

/**
 * recvmsg_safe - safely receive a message from a socket
 * @sockfd: socket file descriptor
 * @msg: pointer to msghdr structure describing the message
 * @flags: flags to pass to recvmsg()
 *
 * Returns:
 *  Number of bytes received on success, 0 if peer closed the connection,
 *  or -errno on failure.
 */
ssize_t recvmsg_safe(int sockfd, struct msghdr *msg, int flags) {
    ssize_t ret;

    if (!msg)
        return -EINVAL;

    ret = recvmsg(sockfd, msg, flags);
    if (ret < 0) {
        int err = errno;
        // Handle transient errors that can be retried
        if (err == EINTR || err == EAGAIN || err == EWOULDBLOCK)
            return 0; // caller can retry
        return -err;
    }

    return ret;
}

int receive_one_fd(int transport_fd, int flags) {
    struct msghdr msg = {0};
    struct iovec iov;
    char buf[1];
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    int received_fd = -1;

    buf[0] = 0;  // dummy data

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);

    ssize_t n = recvmsg(transport_fd, &msg, flags);
    if (n < 0) {
        return -errno;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        memcpy(&received_fd, CMSG_DATA(cmsg), sizeof(int));
    } else {
        return -EBADMSG; // no fd received
    }

    return received_fd;
}

ssize_t next_datagram_size_fd(int fd) {
    char buf;
    ssize_t r;

    /* Peek at the next datagram without removing it from the queue */
    r = recv(fd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0; /* no data available */
        return -1;    /* actual error */
    }

    /* If we peeked 1 byte, try to get the full size using ioctl if available */
#if defined(FIONREAD)
    int n = 0;
    if (ioctl(fd, FIONREAD, &n) == 0)
        return n;
#endif

    /* fallback: return 1 as minimum peek size */
    return r;
}

int netlink_family_from_string(const char *s) {
    if (!s)
        return -1;

    /* macOS has no netlink, so we only support dummy/fallback values */
    if (strcmp(s, "route") == 0)
        return 0;  /* just a placeholder */
    if (strcmp(s, "generic") == 0)
        return 0;  /* placeholder */

    /* unknown family */
    return -1;
}

bool ifname_valid_char(char a) {
    return isalnum((unsigned char)a) || a == '_' || a == '.';
}

bool ifname_valid_full(const char *p, IfnameValidFlags flags) {
    if (!p || !*p)
        return false;

    size_t len = strlen(p);
    if (len >= IFNAMSIZ) // macOS IFNAMSIZ includes null terminator
        return false;

    for (size_t i = 0; i < len; i++) {
        if (!ifname_valid_char(p[i]))
            return false;
    }

    // // Optional: handle flags (example: allow numeric suffix)
    // if (flags & IFNAME_ALLOW_NUMERIC_SUFFIX) {
    //     // already covered by ifname_valid_char, so no extra check needed
    // }

    return true;
}

int getpeersec(int fd, char **ret) {
    if (!ret) return -EINVAL;
    *ret = NULL;

    uid_t uid;
    gid_t gid;
    if (getpeereid(fd, &uid, &gid) < 0)
        return -errno;

    char buf[64];
    int n = snprintf(buf, sizeof(buf), "uid=%u,gid=%u", uid, gid);
    if (n < 0 || n >= (int)sizeof(buf)) return -EIO;

    *ret = strdup(buf);
    if (!*ret) return -ENOMEM;

    return 0;
}

int getpeergroups(int fd, gid_t **ret) {
    if (!ret) return -EINVAL;
    *ret = NULL;

    uid_t uid;
    gid_t gid;
    if (getpeereid(fd, &uid, &gid) < 0)
        return -errno;

    gid_t *groups = malloc(sizeof(gid_t));
    if (!groups) return -ENOMEM;

    groups[0] = gid;
    *ret = groups;
    return 1;  /* number of groups */
}

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
// int socket_address_listen(
//         const SocketAddress *a,
//         int flags,
//         int backlog,
//         SocketAddressBindIPv6Only only,
//         const char *bind_to_device,
//         bool reuse_port,
//         bool free_bind,
//         bool transparent,
//         mode_t directory_mode,
//         mode_t socket_mode,
//         const char *label) {

//     int fd = socket(a->sockaddr.sa.sa_family, a->type, a->protocol);
//     if (fd < 0) return -errno;

//     if (reuse_port) setsockopt_int(fd, SOL_SOCKET, SO_REUSEADDR, 1);

//     if (a->sockaddr.sa.sa_family == AF_UNIX) {
//         char path_copy[sizeof(a->sockaddr.un.sun_path)];

//         strncpy(path_copy, a->sockaddr.un.sun_path, sizeof(path_copy));
//         path_copy[sizeof(path_copy) - 1] = '\0';

//         mkdir(dirname(path_copy), directory_mode);

//         // mkdir(dirname(a->sockaddr.un.sun_path), directory_mode);
//         //sockaddr_un_set_path(&((struct sockaddr_un*) &a->sockaddr)->sun_path, a->sockaddr.un.sun_path);
//         struct sockaddr_un *un = (struct sockaddr_un *) &a->sockaddr;
//         sockaddr_un_set_path(un, un->sun_path);

//         unlink(a->sockaddr.un.sun_path);
//     }

//     if (bind(fd, &a->sockaddr.sa, a->size) < 0) {
//         close(fd);
//         return -errno;
//     }

//     if (listen(fd, backlog) < 0) {
//         close(fd);
//         return -errno;
//     }

//     return fd;
// }

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

ssize_t send_one_fd_iov_sa(
                int transport_fd,
                int fd,
                const struct iovec *iov, size_t iovlen,
                const struct sockaddr *sa, socklen_t len,
                int flags) {

        CMSG_BUFFER_TYPE(CMSG_SPACE(sizeof(int))) control = {};
        struct msghdr mh = {
                .msg_name = (struct sockaddr*) sa,
                .msg_namelen = len,
                .msg_iov = (struct iovec *)iov,
                .msg_iovlen = iovlen,
        };
        ssize_t k;

        assert(transport_fd >= 0);

        /*
         * We need either an FD or data to send.
         * If there's nothing, return an error.
         */
        if (fd < 0 && !iov)
                return -EINVAL;

        if (fd >= 0) {
                struct cmsghdr *cmsg;

                mh.msg_control = &control;
                mh.msg_controllen = sizeof(control);

                cmsg = CMSG_FIRSTHDR(&mh);
                cmsg->cmsg_level = SOL_SOCKET;
                cmsg->cmsg_type = SCM_RIGHTS;
                cmsg->cmsg_len = CMSG_LEN(sizeof(int));
                memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
        }
        k = sendmsg(transport_fd, &mh, MSG_NOSIGNAL | flags);
        if (k < 0)
                return (ssize_t) -errno;

        return k;
}

int send_one_fd_sa(
                int transport_fd,
                int fd,
                const struct sockaddr *sa, socklen_t len,
                int flags) {

        assert(fd >= 0);

        return (int) send_one_fd_iov_sa(transport_fd, fd, NULL, 0, sa, len, flags);
}

int sockaddr_port(const struct sockaddr *_sa, unsigned *ret_port) {
        const union sockaddr_union *sa = (const union sockaddr_union*) _sa;

        /* Note, this returns the port as 'unsigned' rather than 'uint16_t', as AF_VSOCK knows larger ports */

        assert(sa);

        switch (sa->sa.sa_family) {

        case AF_INET:
                *ret_port = be16toh(sa->in.sin_port);
                return 0;

        case AF_INET6:
                *ret_port = be16toh(sa->in6.sin6_port);
                return 0;

        case AF_VSOCK:
                *ret_port = sa->vm.svm_port;
                return 0;

        default:
                return -EAFNOSUPPORT;
        }
}

bool socket_address_can_accept(const SocketAddress *a) {
        assert(a);

        return
                IN_SET(a->type, SOCK_STREAM, SOCK_SEQPACKET);
}

int socket_address_print(const SocketAddress *a, char **ret) {
        int r;

        assert(a);
        assert(ret);

        r = socket_address_verify(a, false); /* We do non-strict validation, because we want to be
                                              * able to pretty-print any socket the kernel considers
                                              * valid. We still need to do validation to know if we
                                              * can meaningfully print the address. */
        if (r < 0)
                return r;

        if (socket_address_family(a) == AF_NETLINK) {
                _cleanup_free_ char *sfamily = NULL;

                r = netlink_family_to_string_alloc(a->protocol, &sfamily);
                if (r < 0)
                        return r;

                r = asprintf(ret, "%s %u", sfamily, a->sockaddr.nl.nl_groups);
                if (r < 0)
                        return -ENOMEM;

                return 0;
        }

        return sockaddr_pretty(&a->sockaddr.sa, a->size, false, true, ret);
}

int socket_address_verify(const SocketAddress *a, bool strict) {
        assert(a);

        /* With 'strict' we enforce additional sanity constraints which are not set by the standard,
         * but should only apply to sockets we create ourselves. */

        switch (socket_address_family(a)) {

        case AF_INET:
                if (a->size != sizeof(struct sockaddr_in))
                        return -EINVAL;

                if (a->sockaddr.in.sin_port == 0)
                        return -EINVAL;

                if (!IN_SET(a->type, 0, SOCK_STREAM, SOCK_DGRAM))
                        return -EINVAL;

                return 0;

        case AF_INET6:
                if (a->size != sizeof(struct sockaddr_in6))
                        return -EINVAL;

                if (a->sockaddr.in6.sin6_port == 0)
                        return -EINVAL;

                if (!IN_SET(a->type, 0, SOCK_STREAM, SOCK_DGRAM))
                        return -EINVAL;

                return 0;

        case AF_UNIX:
                if (a->size < offsetof(struct sockaddr_un, sun_path))
                        return -EINVAL;
                if (a->size > sizeof(struct sockaddr_un) + !strict)
                        /* If !strict, allow one extra byte, since getsockname() on Linux will append
                         * a NUL byte if we have path sockets that are above sun_path's full size. */
                        return -EINVAL;

                if (a->size > offsetof(struct sockaddr_un, sun_path) &&
                    a->sockaddr.un.sun_path[0] != 0 &&
                    strict) {
                        /* Only validate file system sockets here, and only in strict mode */
                        const char *e;

                        e = memchr(a->sockaddr.un.sun_path, 0, sizeof(a->sockaddr.un.sun_path));
                        if (e) {
                                /* If there's an embedded NUL byte, make sure the size of the socket address matches it */
                                if (a->size != offsetof(struct sockaddr_un, sun_path) + (e - a->sockaddr.un.sun_path) + 1)
                                        return -EINVAL;
                        } else {
                                /* If there's no embedded NUL byte, then the size needs to match the whole
                                 * structure or the structure with one extra NUL byte suffixed. (Yeah, Linux is awful,
                                 * and considers both equivalent: getsockname() even extends sockaddr_un beyond its
                                 * size if the path is non NUL terminated.) */
                                if (!IN_SET(a->size, sizeof(a->sockaddr.un.sun_path), sizeof(a->sockaddr.un.sun_path)+1))
                                        return -EINVAL;
                        }
                }

                if (!IN_SET(a->type, 0, SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET))
                        return -EINVAL;

                return 0;

        case AF_NETLINK:

                if (a->size != sizeof(struct sockaddr_nl))
                        return -EINVAL;

                if (!IN_SET(a->type, 0, SOCK_RAW, SOCK_DGRAM))
                        return -EINVAL;

                return 0;

        case AF_VSOCK:
                if (a->size != sizeof(struct sockaddr_vm))
                        return -EINVAL;

                if (!IN_SET(a->type, 0, SOCK_STREAM, SOCK_DGRAM))
                        return -EINVAL;

                return 0;

        default:
                return -EAFNOSUPPORT;
        }
}

int socket_bind_to_ifindex(int fd, int ifindex) {
        char ifname[IF_NAMESIZE];
        int r;

        assert(fd >= 0);

        if (ifindex <= 0)
                /* Drop binding */
                return RET_NERRNO(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, NULL, 0));

        r = setsockopt_int(fd, SOL_SOCKET, SO_BINDTOIFINDEX, ifindex);
        if (r != -ENOPROTOOPT)
                return r;

        /* Fall back to SO_BINDTODEVICE on kernels < 5.0 which didn't have SO_BINDTOIFINDEX */
        if (!if_indextoname(ifindex, ifname)) {
            // handle error
            return -errno;
        }

        return socket_bind_to_ifname(fd, ifname);
}

int socket_bind_to_ifname(int fd, const char *ifname) {
        assert(fd >= 0);

        /* Call with NULL to drop binding */

        return RET_NERRNO(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen_ptr(ifname)));
}

int socket_set_option(int fd, int af, int opt_ipv4, int opt_ipv6, int val) {
        if (af == AF_UNSPEC) {
                af = socket_get_family(fd);
                if (af < 0)
                        return af;
        }

        switch (af) {

        case AF_INET:
                return setsockopt_int(fd, IPPROTO_IP, opt_ipv4, val);

        case AF_INET6:
                return setsockopt_int(fd, IPPROTO_IPV6, opt_ipv6, val);

        default:
                return -EAFNOSUPPORT;
        }
}

int socket_set_unicast_if(int fd, int af, int ifi) {
        be32_t ifindex_be = htobe32(ifi);

        if (af == AF_UNSPEC) {
                af = socket_get_family(fd);
                if (af < 0)
                        return af;
        }

        switch (af) {

        case AF_INET:
                return RET_NERRNO(setsockopt(fd, IPPROTO_IP, IP_UNICAST_IF, &ifindex_be, sizeof(ifindex_be)));

        case AF_INET6:
                return RET_NERRNO(setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_IF, &ifindex_be, sizeof(ifindex_be)));

        default:
                return -EAFNOSUPPORT;
        }
}

/* macOS stub: netlink is Linux-only */
int netlink_family_to_string_alloc(int b, char **s) {
    if (!s)
        return -EINVAL;

    *s = NULL; // no string to return

    (void)b; // silence unused parameter warning

    return -ENOTSUP; // operation not supported
}

int socket_get_family(int fd) {
    if (fd < 0)
        return -EINVAL;

    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (getsockname(fd, (struct sockaddr*)&addr, &len) < 0)
        return -errno; // return negative errno on error

    return addr.ss_family; // AF_INET, AF_INET6, AF_UNIX, etc.
}

