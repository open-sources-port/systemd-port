#pragma once

#ifndef HAVE_VALGRIND_VALGRIND_H

/* Running on Valgrind? Always false on macOS without Valgrind */
#define RUNNING_ON_VALGRIND 0

/* Client request mechanism: no-op */
#define VALGRIND_DO_CLIENT_REQUEST(expr, default, ...) (default)

#endif /* HAVE_VALGRIND_VALGRIND_H */

#ifndef HAVE_VALGRIND_MEMCHECK_H
#define HAVE_VALGRIND_MEMCHECK_H 0
#endif