// compat/signalfd.h
#pragma once

#ifdef __APPLE__

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

typedef struct {
    int signo;
} signalfd_siginfo;

typedef void (*signal_callback_t)(int signo, void *userdata);

struct signalfd_ctx {
    signal_callback_t callback;
    void *userdata;
};

static inline int signalfd(int fd, const sigset_t *mask, int flags) {
    (void)fd; (void)flags;

    // Block signals in the process
    if(sigprocmask(SIG_BLOCK, mask, NULL) < 0) {
        return -1;
    }

    return 0; // success
}

// You need to register signal handler separately:
static inline void signalfd_register_handler(signal_callback_t cb, void *userdata, const sigset_t *mask) {
    struct signalfd_ctx *ctx = malloc(sizeof(*ctx));
    ctx->callback = cb;
    ctx->userdata = userdata;

    // install handler for each signal in mask
    for(int signo = 1; signo < NSIG; signo++) {
        if(sigismember(mask, signo)) {
            struct sigaction sa;
            sa.sa_flags = 0;
            sa.sa_handler = cb ? (void(*)(int))cb : SIG_IGN;
            sigemptyset(&sa.sa_mask);
            sigaction(signo, &sa, NULL);
        }
    }
}

#endif // __APPLE__
