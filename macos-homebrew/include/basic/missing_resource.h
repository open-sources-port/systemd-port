/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <sys/resource.h>

#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif

/* If RLIMIT_RTTIME is not defined, then we cannot use RLIMIT_NLIMITS as is */
#define _RLIMIT_MAX (RLIMIT_RTTIME+1 > RLIMIT_NLIMITS ? RLIMIT_RTTIME+1 : RLIMIT_NLIMITS)

/* macOS does not define these Linux-only limits */
// #ifndef RLIMIT_LOCKS
// #define RLIMIT_LOCKS        (-1)
// #endif

// #ifndef RLIMIT_SIGPENDING
// #define RLIMIT_SIGPENDING  (-1)
// #endif

// #ifndef RLIMIT_MSGQUEUE
// #define RLIMIT_MSGQUEUE    (-1)
// #endif

// #ifndef RLIMIT_NICE
// #define RLIMIT_NICE        (-1)
// #endif

// #ifndef RLIMIT_RTPRIO
// #define RLIMIT_RTPRIO      (-1)
// #endif


/* macOS does NOT define RLIMIT_NLIMITS */
#if defined(__APPLE__)
#  define RLIMIT_NLIMITS RLIM_NLIMITS
#endif

/* systemd expects RLIMIT_RTTIME, but macOS doesn't have it */
#ifndef RLIMIT_RTTIME
#  define RLIMIT_RTTIME (RLIMIT_NLIMITS - 1)
#endif

// #define _RLIMIT_MAX RLIMIT_NLIMITS