#ifndef PORTABLE_STRINGIFY_H
#define PORTABLE_STRINGIFY_H

/* STRINGIFY: Convert macro argument to string literal after expansion */
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

/* WSTRINGIFY: Convert macro argument to wide string literal */
#ifdef __cplusplus
  #define WSTRINGIFY_HELPER(x) L#x
  #define WSTRINGIFY(x) WSTRINGIFY_HELPER(x)
#elif defined(_MSC_VER) && defined(_UNICODE)
  #define WSTRINGIFY_HELPER(x) L#x
  #define WSTRINGIFY(x) WSTRINGIFY_HELPER(x)
#else
  #define WSTRINGIFY(x) STRINGIFY(x)
#endif

/* CONCAT: Concatenate two tokens after expansion */
#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)

/* CONCAT3: Concatenate three tokens */
#define CONCAT3_HELPER(a, b, c) a##b##c
#define CONCAT3(a, b, c) CONCAT3_HELPER(a, b, c)

/* DEBUG_STRING: Useful for debugging macros to see expanded value */
#define DEBUG_STRING(x) "DEBUG: " #x " = " STRINGIFY(x)

/* UNUSED: Mark variable as unused to avoid compiler warnings */
#if defined(__GNUC__) || defined(__clang__)
  #define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
  #define UNUSED(x) x
#endif

#define __stringify(x) STRINGIFY(x)
#define __PASTE(a,b) CONCAT(a,b)
#define __PASTE3(a,b,c) CONCAT3(a,b,c)

#endif /* PORTABLE_STRINGIFY_H */
