#pragma once

#include <unistd.h>

#if defined(__APPLE__)

#include <pthread.h>
#include <stdint.h>

/* macOS replacement for gettid() */
static inline pid_t gettid(void) {
    uint64_t tid;
    if (pthread_threadid_np(NULL, &tid) != 0) {
        return (pid_t) -1;  // fallback on error
    }
    return (pid_t) tid;
}

#endif
