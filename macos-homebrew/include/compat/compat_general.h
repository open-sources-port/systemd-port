#include <stdlib.h>
#include <unistd.h>

/*
 * macOS does not provide secure_getenv or __secure_getenv.
 * We emulate them by checking if the process is running setuid/setgid.
 * If so, return NULL. Otherwise, return getenv(name).
 */

static inline char *secure_getenv(const char *name) {
    if (geteuid() != getuid() || getegid() != getgid()) {
        // running with elevated privileges → ignore environment
        return NULL;
    }
    return getenv(name);
}

static inline char *__secure_getenv(const char *name) {
    return secure_getenv(name);  // alias to the same implementation
}
