// time_types_compat.h
#pragma once

#include <time.h>
#include <sys/time.h>

// Linux defines suseconds_t in <bits/types.h>, macOS already has it
typedef suseconds_t suseconds_t;

typedef struct timeval timeval;
typedef struct timespec timespec;
typedef struct itimerspec itimerspec;
