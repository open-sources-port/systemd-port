#ifndef _PORTABLE_INIT_H
#define _PORTABLE_INIT_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/compiler.h>
#include <linux/stringify.h>

#ifdef __cplusplus
extern "C" {
#endif

// Registration macro
#define REGISTER_INITCALL(fn) \
    static int fn(void); \
    static initcall_t __initcall_##fn __attribute__((used, section("__DATA,__initcalls"))) = fn
typedef int (*initcall_t)(void);

#if defined(__linux__) || defined(__APPLE__)
  #define SECTION_ATTR(name) __attribute__((section(name), used))
  extern initcall_t __start_initcall0[] __attribute__((weak));
  extern initcall_t __stop_initcall0[] __attribute__((weak));
#elif defined(_MSC_VER)
  #define SECTION_ATTR(name) __declspec(allocate(name))
  __declspec(selectany) initcall_t *__start_initcall0 = NULL;
  __declspec(selectany) initcall_t *__stop_initcall0 = NULL;
#else
  #define SECTION_ATTR(name) __attribute__((section(name), used))
#endif

#if defined(_MSC_VER)
  extern initcall_t __start_initcalls;
  extern initcall_t __stop_initcalls;

  #define DEFINE_INITCALL(fn) \
      __declspec(allocate("INIT$M")) initcall_t __initcall_##fn = fn;
#else
  typedef int (*initcall_t)(void);

  #define __define_initcall(fn, id) \
      static initcall_t __initcall_##fn __attribute__((used, section(".initcall" #id ".init"))) = fn;

#endif

#define early_initcall(fn) __define_initcall(fn, 0)

int do_one_initcall(initcall_t fn);
int run_initcalls(initcall_t *start, initcall_t *end);
int run_early_initcalls(void);

#ifdef __cplusplus
}
#endif

#endif /* _PORTABLE_INIT_H */
