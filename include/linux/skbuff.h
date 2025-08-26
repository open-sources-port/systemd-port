/* SPDX-License-Identifier: MIT */
/*
 * Cross-platform replacement for <linux/skbuff.h>
 * Works on Windows and macOS. Provides API-compatible stubs
 * so open source projects compile without modification.
 *
 * NOTE: This is NOT a full networking stack. Functions are
 * simplified, buffers are heap allocated, and many advanced
 * kernel features are no-ops.
 */

#ifndef __LINUX_SKBUFF_H
#define __LINUX_SKBUFF_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Basic typedefs */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;

/* Fake refcount */
typedef int refcount_t;
static inline void refcount_inc(refcount_t *r) { if (r) (*r)++; }
static inline int refcount_read(const refcount_t *r) { return r ? *r : 1; }

/* GFP flags (dummy values) */
#define GFP_ATOMIC 0
#define GFP_KERNEL 0

/* checksum type */
typedef uint16_t __sum16;

/* Fragment definitions */
#define CONFIG_MAX_SKB_FRAGS 17
#define MAX_SKB_FRAGS CONFIG_MAX_SKB_FRAGS

typedef struct skb_frag {
    void *netmem;
    unsigned int len;
    unsigned int offset;
} skb_frag_t;

static inline unsigned int skb_frag_size(const skb_frag_t *f) { return f->len; }
static inline void skb_frag_size_set(skb_frag_t *f, unsigned int s) { f->len = s; }

/* Shared info */
struct skb_shared_info {
    u8 flags;
    u8 nr_frags;
    u16 gso_size;
    u16 gso_segs;
    struct sk_buff *frag_list;
    u32 gso_type;
    refcount_t dataref;
    void *destructor_arg;
    skb_frag_t frags[MAX_SKB_FRAGS];
};

/* Main sk_buff struct */
struct sk_buff {
    struct sk_buff *next;
    struct sk_buff *prev;

    void *dev; /* opaque net_device */
    void *sk;  /* opaque sock */

    unsigned int len;
    unsigned int data_len;
    unsigned int truesize;

    unsigned char *head;
    unsigned char *data;
    unsigned char *tail;
    unsigned char *end;

    refcount_t users;
    struct skb_shared_info *shinfo;
    void *private;
};

/* Helpers */
static inline struct skb_shared_info *skb_shinfo(const struct sk_buff *skb) {
    return skb ? skb->shinfo : NULL;
}

/* Alloc/free */
static inline struct sk_buff *__alloc_skb(unsigned int size) {
    struct sk_buff *skb = (struct sk_buff *)malloc(sizeof(*skb));
    if (!skb) return NULL;
    memset(skb, 0, sizeof(*skb));
    skb->head = (unsigned char *)malloc(size);
    if (!skb->head) { free(skb); return NULL; }
    skb->data = skb->tail = skb->head;
    skb->end = skb->head + size;
    skb->len = 0;
    skb->truesize = size + sizeof(*skb);
    skb->users = 1;
    return skb;
}

static inline struct sk_buff *alloc_skb(unsigned int size, int priority) {
    (void)priority;
    return __alloc_skb(size);
}

static inline void kfree_skb(struct sk_buff *skb) {
    if (!skb) return;
    if (skb->head) free(skb->head);
    free(skb);
}
#define consume_skb kfree_skb
#define dev_kfree_skb kfree_skb
#define dev_kfree_skb_any kfree_skb
#define dev_kfree_skb_irq kfree_skb

/* Data manipulation */
static inline unsigned char *skb_put(struct sk_buff *skb, unsigned int len) {
    unsigned char *tmp = skb->tail;
    skb->tail += len;
    skb->len += len;
    return tmp;
}
static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len) {
    skb->data -= len; skb->len += len; return skb->data;
}
static inline unsigned char *skb_pull(struct sk_buff *skb, unsigned int len) {
    skb->data += len; skb->len -= len; return skb->data;
}
static inline void skb_reserve(struct sk_buff *skb, unsigned int len) {
    skb->data += len; skb->tail = skb->data;
}

static inline unsigned int skb_headroom(const struct sk_buff *skb) { return skb->data - skb->head; }
static inline unsigned int skb_tailroom(const struct sk_buff *skb) { return skb->end - skb->tail; }
static inline unsigned int skb_headlen(const struct sk_buff *skb) { return skb->len; }

/* Copy/clone */
static inline struct sk_buff *skb_clone(const struct sk_buff *skb) {
    if (!skb) return NULL;
    struct sk_buff *n = (struct sk_buff *)malloc(sizeof(*n));
    if (!n) return NULL;
    *n = *skb; /* shallow copy */
    refcount_inc(&n->users);
    return n;
}
static inline struct sk_buff *skb_copy(const struct sk_buff *skb) {
    if (!skb) return NULL;
    struct sk_buff *n = __alloc_skb(skb->end - skb->head);
    if (!n) return NULL;
    memcpy(n->head, skb->head, skb->end - skb->head);
    n->data = n->head + (skb->data - skb->head);
    n->tail = n->head + (skb->tail - skb->head);
    n->len  = skb->len;
    return n;
}

/* Misc stubs */
static inline void skb_reset_mac_header(struct sk_buff *skb) { (void)skb; }
static inline void skb_reset_network_header(struct sk_buff *skb) { (void)skb; }
static inline void skb_reset_transport_header(struct sk_buff *skb) { (void)skb; }

#endif /* __LINUX_SKBUFF_H */
