// compat/gshadow.h
#pragma once

#ifdef __APPLE__
#include <stddef.h>

// Minimal struct to satisfy code expecting gshadow
struct spwd {
    char *sp_namp;    // login name
    char *sp_pwdp;    // encrypted password
    long  sp_lstchg;  // date of last change
    long  sp_min;     // min days between changes
    long  sp_max;     // max days between changes
    long  sp_warn;    // days before password expires
    long  sp_inact;   // days after expiration until account disabled
    long  sp_expire;  // account expiration date
    unsigned long sp_flag;
};

// Stub functions for getspnam / set spent functions
static inline struct spwd *getspnam(const char *name) {
    (void)name;
    return NULL;
}

static inline int setspent(void) { return 0; }
static inline void endspent(void) {}
static inline void setspent_r() {}
#endif
