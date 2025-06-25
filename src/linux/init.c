#include "linux/init.h"
#include "linux/types.h"
#include <stdio.h>

int do_one_initcall(initcall_t fn) {
    if (!fn) return -1;
    return fn();
}

int run_initcalls(initcall_t *start, initcall_t *end) {
    int ret = 0;
    for (initcall_t *fn = start; fn < end; ++fn) {
        if (*fn) {
            int r = do_one_initcall(*fn);
            if (r != 0) {
                fprintf(stderr, "Initcall failed: %p\n", (void *)*fn);
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

#elif defined(_MSC_VER)

#pragma section("INIT$A", read)
#pragma section("INIT$Z", read)

__declspec(allocate("INIT$A")) initcall_t __start_initcalls = 0;
__declspec(allocate("INIT$Z")) initcall_t __stop_initcalls = 0;

#pragma comment(linker, "/merge:INIT=.rdata")

extern initcall_t __start_initcalls;
extern initcall_t __stop_initcalls;

int run_early_initcalls(void) {
    return run_initcalls(&__start_initcalls, &__stop_initcalls);
}

#elif defined(__linux__)

extern initcall_t __start_initcalls[];
extern initcall_t __stop_initcalls[];

int run_early_initcalls(void) {
    return run_initcalls(__start_initcalls, __stop_initcalls);
}

#else

int run_early_initcalls(void) {
    return 0;
}

#endif
