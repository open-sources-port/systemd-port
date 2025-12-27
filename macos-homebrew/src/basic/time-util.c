/* SPDX-License-Identifier: LGPL-2.1-or-later */

#define _DARWIN_C_SOURCE

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <linux/rtc.h>

#include "time-util.h"
#include "errno-util.h"
#include <basic/macro.h>

char* format_timestamp_style(
                char *buf,
                size_t l,
                usec_t t,
                TimestampStyle style) {

        struct tm tm;
        time_t sec;
        int r;

        if (!buf || l == 0)
                return NULL;

        if (t == USEC_INFINITY)
                return snprintf(buf, l, "infinity") < (int) l ? buf : NULL;

        if (t == 0)
                return snprintf(buf, l, "n/a") < (int) l ? buf : NULL;

        if (style == TIMESTAMP_US) {
                r = snprintf(buf, l, "%" PRIu64, t);
                return (r >= 0 && (size_t) r < l) ? buf : NULL;
        }

        sec = (time_t) (t / USEC_PER_SEC);

        if (style == TIMESTAMP_UTC) {
                if (!gmtime_r(&sec, &tm))
                        return NULL;

                r = strftime(buf, l, "%Y-%m-%dT%H:%M:%SZ", &tm);
        } else {
                if (!localtime_r(&sec, &tm))
                        return NULL;

                if (style == TIMESTAMP_PRETTY)
                        r = strftime(buf, l, "%a %Y-%m-%d %H:%M:%S", &tm);
                else /* TIMESTAMP_LOCAL */
                        r = strftime(buf, l, "%Y-%m-%d %H:%M:%S", &tm);
        }

        return r > 0 ? buf : NULL;
}

static inline usec_t now_usec(void) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (usec_t) tv.tv_sec * USEC_PER_SEC + tv.tv_usec;
}

char* format_timestamp_relative(
                char *buf,
                size_t l,
                usec_t t) {

        usec_t n, d;
        char tmp[64];

        if (!buf || l == 0)
                return NULL;

        if (t == USEC_INFINITY)
                return snprintf(buf, l, "infinity") < (int) l ? buf : NULL;

        n = now_usec();

        if (t == n)
                return snprintf(buf, l, "now") < (int) l ? buf : NULL;

        if (t > n) {
                d = t - n;

                if (!format_timespan(tmp, sizeof tmp, d, USEC_PER_SEC))
                        return NULL;

                return snprintf(buf, l, "in %s", tmp) < (int) l ? buf : NULL;
        } else {
                d = n - t;

                if (!format_timespan(tmp, sizeof tmp, d, USEC_PER_SEC))
                        return NULL;

                return snprintf(buf, l, "%s ago", tmp) < (int) l ? buf : NULL;
        }
}

/* ================================================================
 * macOS clock model
 *
 * CLOCK_REALTIME   -> wall clock
 * CLOCK_MONOTONIC  -> monotonic
 * CLOCK_BOOTTIME   -> mapped to monotonic
 * *_ALARM clocks   -> unsupported
 * ================================================================ */

static clockid_t normalize_clock(clockid_t clock) {
        switch (clock) {
        case CLOCK_REALTIME:
                return CLOCK_REALTIME;

        case CLOCK_MONOTONIC:
                return CLOCK_MONOTONIC;

        default:
                return (clockid_t) -1;
        }
}

bool clock_supported(clockid_t clock) {
        return IN_SET(clock,
                      CLOCK_REALTIME,
                      CLOCK_MONOTONIC,
                      CLOCK_BOOTTIME);
}

/* ================================================================
 * now()
 * ================================================================ */

usec_t now(clockid_t clock) {
        struct timespec ts = {};
        clockid_t c = normalize_clock(clock);

        if (c < 0)
                return USEC_INFINITY;

        if (clock_gettime(c, &ts) < 0)
                return USEC_INFINITY;

        return timespec_load(&ts);
}

nsec_t now_nsec(clockid_t clock) {
        struct timespec ts = {};
        clockid_t c = normalize_clock(clock);

        if (c < 0)
                return NSEC_INFINITY;

        if (clock_gettime(c, &ts) < 0)
                return NSEC_INFINITY;

        return timespec_load_nsec(&ts);
}

/* ================================================================
 * time mapping
 * ================================================================ */

usec_t map_clock_usec(usec_t from, clockid_t from_clock, clockid_t to_clock) {
        clockid_t fc = normalize_clock(from_clock);
        clockid_t tc = normalize_clock(to_clock);

        if (fc < 0 || tc < 0)
                return USEC_INFINITY;

        if (fc == tc)
                return from;

        usec_t now_from = now(fc);
        usec_t now_to   = now(tc);

        if (now_from == USEC_INFINITY || now_to == USEC_INFINITY)
                return USEC_INFINITY;

        if (from > now_from)
                return USEC_INFINITY;

        return now_to - (now_from - from);
}

/* ================================================================
 * dual / triple timestamps
 * ================================================================ */

dual_timestamp* dual_timestamp_get(dual_timestamp *ts) {
        assert(ts);

        ts->realtime  = now(CLOCK_REALTIME);
        ts->monotonic = now(CLOCK_MONOTONIC);

        return ts;
}

dual_timestamp* dual_timestamp_from_realtime(dual_timestamp *ts, usec_t u) {
        assert(ts);

        ts->realtime  = u;
        ts->monotonic = map_clock_usec(u, CLOCK_REALTIME, CLOCK_MONOTONIC);

        return ts;
}

dual_timestamp* dual_timestamp_from_monotonic(dual_timestamp *ts, usec_t u) {
        assert(ts);

        ts->monotonic = u;
        ts->realtime  = map_clock_usec(u, CLOCK_MONOTONIC, CLOCK_REALTIME);

        return ts;
}

dual_timestamp* dual_timestamp_from_boottime(dual_timestamp *ts, usec_t u) {
        /* boottime == monotonic on macOS */
        return dual_timestamp_from_monotonic(ts, u);
}

triple_timestamp* triple_timestamp_get(triple_timestamp *ts) {
        assert(ts);

        ts->realtime  = now(CLOCK_REALTIME);
        ts->monotonic = now(CLOCK_MONOTONIC);
        ts->boottime  = ts->monotonic;

        return ts;
}

triple_timestamp* triple_timestamp_from_realtime(triple_timestamp *ts, usec_t u) {
        assert(ts);

        ts->realtime  = u;
        ts->monotonic = map_clock_usec(u, CLOCK_REALTIME, CLOCK_MONOTONIC);
        ts->boottime  = ts->monotonic;

        return ts;
}

usec_t triple_timestamp_by_clock(triple_timestamp *ts, clockid_t clock) {
        assert(ts);

        switch (clock) {
        case CLOCK_REALTIME:
                return ts->realtime;

        case CLOCK_MONOTONIC:
                return ts->monotonic;

        default:
                return USEC_INFINITY;
        }
}

/* ================================================================
 * timespec / timeval helpers
 * ================================================================ */

usec_t timespec_load(const struct timespec *ts) {
        assert(ts);
        return (usec_t) ts->tv_sec * USEC_PER_SEC +
               (usec_t) ts->tv_nsec / NSEC_PER_USEC;
}

nsec_t timespec_load_nsec(const struct timespec *ts) {
        assert(ts);
        return (nsec_t) ts->tv_sec * NSEC_PER_SEC + (nsec_t) ts->tv_nsec;
}

struct timespec* timespec_store(struct timespec *ts, usec_t u) {
        assert(ts);

        ts->tv_sec  = (time_t) (u / USEC_PER_SEC);
        ts->tv_nsec = (long) ((u % USEC_PER_SEC) * NSEC_PER_USEC);

        return ts;
}

struct timespec* timespec_store_nsec(struct timespec *ts, nsec_t n) {
        assert(ts);

        ts->tv_sec  = (time_t) (n / NSEC_PER_SEC);
        ts->tv_nsec = (long) (n % NSEC_PER_SEC);

        return ts;
}

usec_t timeval_load(const struct timeval *tv) {
        assert(tv);
        return (usec_t) tv->tv_sec * USEC_PER_SEC + (usec_t) tv->tv_usec;
}

struct timeval* timeval_store(struct timeval *tv, usec_t u) {
        assert(tv);

        tv->tv_sec  = (time_t) (u / USEC_PER_SEC);
        tv->tv_usec = (suseconds_t) (u % USEC_PER_SEC);

        return tv;
}

/* ================================================================
 * timezone helpers (macOS)
 * ================================================================ */

struct tm *localtime_or_gmtime_r(const time_t *t, struct tm *tm, bool utc) {
        assert(t);
        assert(tm);

        return utc ? gmtime_r(t, tm) : localtime_r(t, tm);
}

time_t mktime_or_timegm(struct tm *tm, bool utc) {
        assert(tm);
        return utc ? timegm(tm) : mktime(tm);
}

bool in_utc_timezone(void) {
        time_t t = 0;
        struct tm lt = {}, gt = {};

        localtime_r(&t, &lt);
        gmtime_r(&t, &gt);

        return lt.tm_hour == gt.tm_hour;
}

/* ================================================================
 * unsupported Linux-only features
 * ================================================================ */

int time_change_fd(void) {
        return -EOPNOTSUPP;
}

uint32_t usec_to_jiffies(usec_t usec) {
        /* no jiffies on macOS */
        return (uint32_t) (usec / USEC_PER_MSEC);
}

usec_t jiffies_to_usec(uint32_t jiffies) {
        return (usec_t) jiffies * USEC_PER_MSEC;
}
