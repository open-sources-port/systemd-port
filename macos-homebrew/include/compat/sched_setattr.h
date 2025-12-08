#pragma once

#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>

/* Approximate sched_setattr using pthread_setschedparam */
static inline int sched_setattr(pid_t pid, const struct sched_param *param, unsigned int flags) {
    (void)flags;

    pthread_t thread;
    if (pid == 0) {
        thread = pthread_self();
    } else {
        // Cannot map arbitrary pid to pthread_t on macOS
        errno = ENOSYS;
        return -1;
    }

    int policy = param->sched_priority ? SCHED_RR : SCHED_OTHER;

    if (pthread_setschedparam(thread, policy, param) != 0) {
        return -1;  // errno is set by pthread_setschedparam
    }

    return 0;
}
