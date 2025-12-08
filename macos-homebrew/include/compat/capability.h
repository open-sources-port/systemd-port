#pragma once
// macOS stub for libcap
#define cap_t void*
#define cap_value_t int
static inline cap_t cap_get_proc(void) { return NULL; }
static inline int cap_set_proc(cap_t c) { return 0; }
static inline void cap_free(void *c) { }
