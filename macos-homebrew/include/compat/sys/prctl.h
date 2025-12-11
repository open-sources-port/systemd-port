#pragma once
/* compat/sys/prctl.h
 *
 * Stub for macOS to allow Linux code using <sys/prctl.h> to compile.
 * All functions are no-op or return dummy values.
 */

#ifdef __APPLE__

/* prctl options (common ones used in Linux) */
#define PR_SET_NAME 15
#define PR_GET_NAME 16
#define PR_SET_DUMPABLE 4

/* Dummy prctl function */
static inline int prctl(int option, unsigned long arg2, unsigned long arg3,
                        unsigned long arg4, unsigned long arg5) {
    (void)option; (void)arg2; (void)arg3; (void)arg4; (void)arg5;
    return 0; /* pretend success */
}

#endif /* __APPLE__ */
