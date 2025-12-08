#pragma once

#include <sys/resource.h>
#include <errno.h>

static inline int ioprio_get(int which, int who) {
    (void)which;
    int prio = getpriority(PRIO_PROCESS, who);
    if (prio == -1 && errno) return -1;
    return prio;  // approximate
}

static inline int ioprio_set(int which, int who, int ioprio) {
    (void)which;
    // Map ioprio 0..7 to nice values 0..20 (roughly)
    int nice_val = (20 * ioprio) / 7;
    if (setpriority(PRIO_PROCESS, who, nice_val) == -1)
        return -1;
    return 0;
}
