#include "linux/init.h"
#include <stdio.h>

int do_one_initcall(initcall_t fn) {
    if (!fn) return -1;
    return fn();
}

int run_initcalls(initcall_t *start, initcall_t *end) {
    int ret = 0;
    for (initcall_t *fn = start; fn != end; ++fn) {
        if (*fn) {
            int r = do_one_initcall(*fn);
            if (r != 0) {
                fprintf(stderr, "Initcall failed: %p\n", (void*)*fn);
                ret = r;
            }
        }
    }
    return ret;
}

#if defined(__APPLE__)
#include <mach-o/getsect.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>

extern const struct mach_header_64 _mh_execute_header;

int run_early_initcalls(void) {
    unsigned long size = 0;
    initcall_t *start = (initcall_t *)getsectiondata(&_mh_execute_header, "__DATA", "__initcalls", &size);
    if (!start || size == 0) return 0;
    initcall_t *end = (initcall_t *)((char *)start + size);
    return run_initcalls(start, end);
}

#elif defined(__linux__)

// These symbols are created automatically by the linker
extern initcall_t __start___initcalls[];
extern initcall_t __stop___initcalls[];

int run_early_initcalls(void) {
    return run_initcalls(__start___initcalls, __stop___initcalls);
}

#elif defined(_MSC_VER)

extern initcall_t __start_initcalls[];
extern initcall_t __stop_initcalls[];

int run_early_initcalls(void) {
    return run_initcalls(__start_initcalls, __stop_initcalls);
}

#else

int run_early_initcalls(void) {
    return 0; // Unsupported platform
}

#endif
