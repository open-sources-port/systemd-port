#pragma once

#ifdef __APPLE__
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* Linux signals that macOS does not define */
#ifndef SIGPWR
#define SIGPWR 0
#endif

#ifndef SIGRTMIN
#define SIGRTMIN 0
#endif

#ifndef SIGRTMAX
#define SIGRTMAX 0
#endif

static inline int sigisemptyset(const sigset_t *set) {
    sigset_t empty;
    sigemptyset(&empty);
    return memcmp(set, &empty, sizeof(sigset_t)) == 0;
}

/* Provide a fallback for sigtimedwait (not available on macOS) */
static inline int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout) {
    (void)set;
    (void)info;
    (void)timeout;
    errno = ENOSYS; /* Function not implemented */
    return -1;
}

#endif
