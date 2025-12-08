#pragma once

#ifndef HAVE_VALGRIND_MEMCHECK_H

/* Memory annotations become no-ops */
#define VALGRIND_MAKE_MEM_DEFINED(addr, len)     ((void)0)
#define VALGRIND_MAKE_MEM_UNDEFINED(addr, len)   ((void)0)
#define VALGRIND_MAKE_MEM_NOACCESS(addr, len)    ((void)0)
#define VALGRIND_CHECK_MEM_IS_DEFINED(addr, len) (0)
#define VALGRIND_CHECK_MEM_IS_ADDRESSABLE(addr, len) (0)

#endif /* HAVE_VALGRIND_MEMCHECK_H */
