#include <linux/init.h>
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

int run_early_initcalls(void) {
#if defined(__linux__) || defined(__APPLE__)
    if (&__start_initcall0 && &__stop_initcall0)
        return run_initcalls(__start_initcall0, __stop_initcall0);
    return 0;
#elif defined(_MSC_VER)
    if (__start_initcall0 && __stop_initcall0)
        return run_initcalls(__start_initcall0, __stop_initcall0);
    return 0;
#else
    return 0; /* unsupported */
#endif
}
