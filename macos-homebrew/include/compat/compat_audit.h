// compat_audit.h
#pragma once

#if defined(__APPLE__)

/*
 * Stub replacement for <linux/audit.h>
 * Only defines enough symbols so systemd/libudev can compile on macOS.
 * This does NOT provide real Linux audit functionality.
 */
#include <stdint.h>

// macOS / FreeBSD audit API
#include <bsm/audit.h> 

// ------------------------------------------------------------
// Message types (subset used by systemd)
// ------------------------------------------------------------
#define AUDIT_GET          1000    /* Get status */
#define AUDIT_SET          1001    /* Set status */
#define AUDIT_LIST         1002    /* List rules */
#define AUDIT_ADD          1003    /* Add rule */
#define AUDIT_DEL          1004    /* Delete rule */
#define AUDIT_SIGNAL_INFO  1012    /* Get info about sender */

// ------------------------------------------------------------
// Arch tokens (for identifying syscall ABI)
// ------------------------------------------------------------
#define AUDIT_ARCH_X86_64  0xC000003E
#define AUDIT_ARCH_I386    0x40000003
#define AUDIT_ARCH_ARM64   0xC00000B7

// ------------------------------------------------------------
// Status structure
// ------------------------------------------------------------
struct audit_status {
    uint32_t mask;
    uint32_t enabled;
    uint32_t failure;
    uint32_t pid;
    uint32_t rate_limit;
    uint32_t backlog_limit;
    uint32_t lost;
    uint32_t backlog;
};

// ------------------------------------------------------------
// Simple rule data structure (empty stub)
// ------------------------------------------------------------
struct audit_rule_data {
    uint32_t flags;
    uint32_t action;
    uint32_t field_count;
    uint32_t mask[64];
    uint32_t fields[64];
    uint32_t values[64];
    uint32_t fieldflags[64];
    uint32_t buflen;
    char buf[0];
};

#endif // __APPLE__
