#ifndef COMPAT_LINUX_AUDIT_H
#define COMPAT_LINUX_AUDIT_H

#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t, uint64_t etc
#include <unistd.h>  // ssize_t, gid_t, uid_t
#include <errno.h>   // errno constants
#include <bsm/audit.h>

#define AUDIT_FEATURE_ARG_LEN 1024
#define AUDIT_STATUS_ENABLED 1
#define AUDIT_GET_FEATURE 0xBEEF

// typedef int32_t au_event_t; // placeholder type
typedef int audit_fd_t;     // fake fd type

struct audit_status {
    uint32_t enabled;
    uint32_t failure;
    uint32_t pid;
    uint32_t rate_limit;
    uint32_t backlog_limit;
};

static inline int audit_open(void) {
    // No-op on macOS
    return -1;
}

static inline int audit_send(int fd, void *msg, size_t size) {
    // No-op
    (void)fd;
    (void)msg;
    (void)size;
    return -1;
}

static inline int audit_log_user_message(int fd, au_event_t event,
                                         const char *msg, uint32_t len,
                                         uint32_t *sessionid) {
    (void)fd;
    (void)event;
    (void)msg;
    (void)len;
    (void)sessionid;
    return -1;
}

#endif // COMPAT_LINUX_AUDIT_H
