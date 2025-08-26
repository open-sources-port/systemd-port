/* SPDX-License-Identifier: MIT */
/*
 * Cross-platform replacement for <linux/net.h>
 * Works on Windows and macOS. Provides API-compatible stubs
 * so open source projects compile without modification.
 */

#ifndef _LINUX_NET_H
#define _LINUX_NET_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ---- Socket flags ---- */
enum socket_flags {
    SOCKWQ_ASYNC_NOSPACE,
    SOCKWQ_ASYNC_WAITDATA,
    SOCK_NOSPACE,
    SOCK_SUPPORT_ZC,
    SOCK_CUSTOM_SOCKOPT,
};

/* ---- Socket types ---- */
enum sock_type {
    SOCK_STREAM   = 1,
    SOCK_DGRAM    = 2,
    SOCK_RAW      = 3,
    SOCK_RDM      = 4,
    SOCK_SEQPACKET= 5,
    SOCK_DCCP     = 6,
    SOCK_PACKET   = 10,
};
#define SOCK_MAX (SOCK_PACKET + 1)
#define SOCK_TYPE_MASK 0xf

/* Flags for socket, socketpair, accept4 */
#define SOCK_CLOEXEC   0x1
#define SOCK_NONBLOCK  0x2
#define SOCK_COREDUMP  0x4

/* ---- Shutdown commands ---- */
enum sock_shutdown_cmd {
    SHUT_RD,
    SHUT_WR,
    SHUT_RDWR,
};

/* Forward decls */
struct sk_buff;
struct sock;
struct sockaddr;
struct msghdr;
struct kvec;
struct file;
struct module;

/* ---- Wait queue stub ---- */
struct socket_wq {
    unsigned long flags;
};

/* ---- Socket structure ---- */
typedef int socket_state;
struct socket {
    socket_state       state;
    short              type;
    unsigned long      flags;
    struct file       *file;
    struct sock       *sk;
    const struct proto_ops *ops;
    struct socket_wq   wq;
};

/* ---- Read descriptor ---- */
typedef struct {
    size_t written;
    size_t count;
    union {
        char *buf;
        void *data;
    } arg;
    int error;
} read_descriptor_t;

/* ---- Proto ops ---- */
typedef int (*sk_read_actor_t)(read_descriptor_t *, struct sk_buff *, unsigned int, size_t);
typedef int (*skb_read_actor_t)(struct sock *, struct sk_buff *);

struct proto_ops {
    int  family;
    struct module *owner;
    int  (*release)(struct socket *sock);
    int  (*bind)(struct socket *sock, struct sockaddr *addr, int addrlen);
    int  (*connect)(struct socket *sock, struct sockaddr *addr, int addrlen, int flags);
    int  (*socketpair)(struct socket *sock1, struct socket *sock2);
    int  (*accept)(struct socket *sock, struct socket *newsock, void *arg);
    int  (*getname)(struct socket *sock, struct sockaddr *addr, int peer);
    int  (*listen)(struct socket *sock, int backlog);
    int  (*shutdown)(struct socket *sock, int flags);
    int  (*setsockopt)(struct socket *sock, int level, int optname, const void *optval, unsigned int optlen);
    int  (*getsockopt)(struct socket *sock, int level, int optname, void *optval, int *optlen);
    int  (*sendmsg)(struct socket *sock, struct msghdr *m, size_t len);
    int  (*recvmsg)(struct socket *sock, struct msghdr *m, size_t len, int flags);
};

/* ---- Net proto family ---- */
struct net;
struct net_proto_family {
    int family;
    int (*create)(struct net *net, struct socket *sock, int protocol, int kern);
    struct module *owner;
};

/* ---- Function stubs ---- */
static inline int sock_register(const struct net_proto_family *fam) { (void)fam; return 0; }
static inline void sock_unregister(int family) { (void)family; }
static inline bool sock_is_registered(int family) { (void)family; return true; }

static inline int sock_create(int family, int type, int proto, struct socket **res) {
    (void)family; (void)type; (void)proto; *res = NULL; return 0;
}
static inline void sock_release(struct socket *sock) { (void)sock; }

static inline int sock_sendmsg(struct socket *sock, struct msghdr *msg) { (void)sock; (void)msg; return 0; }
static inline int sock_recvmsg(struct socket *sock, struct msghdr *msg, int flags) { (void)sock; (void)msg; (void)flags; return 0; }

static inline int kernel_bind(struct socket *sock, struct sockaddr *addr, int addrlen) { (void)sock; (void)addr; (void)addrlen; return 0; }
static inline int kernel_listen(struct socket *sock, int backlog) { (void)sock; (void)backlog; return 0; }
static inline int kernel_accept(struct socket *sock, struct socket **newsock, int flags) { (void)sock; (void)newsock; (void)flags; return 0; }
static inline int kernel_connect(struct socket *sock, struct sockaddr *addr, int addrlen, int flags) { (void)sock; (void)addr; (void)addrlen; (void)flags; return 0; }
static inline int kernel_getsockname(struct socket *sock, struct sockaddr *addr) { (void)sock; (void)addr; return 0; }
static inline int kernel_getpeername(struct socket *sock, struct sockaddr *addr) { (void)sock; (void)addr; return 0; }
static inline int kernel_sock_shutdown(struct socket *sock, enum sock_shutdown_cmd how) { (void)sock; (void)how; return 0; }

#endif /* _LINUX_NET_H */
