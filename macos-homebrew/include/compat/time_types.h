#pragma once

#ifndef __linux__
#include <time.h>

/* Map Linux __kernel_timespec to POSIX timespec */
struct __kernel_timespec {
    time_t tv_sec;
    long   tv_nsec;
};

struct __kernel_itimerspec {
    struct __kernel_timespec it_interval;
    struct __kernel_timespec it_value;
};

#endif /* __linux__ */
