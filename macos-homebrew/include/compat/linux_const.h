// compat_const.h
#pragma once

// Match Linux <linux/const.h> style
#define _AC(X,Y)   (X##Y)
#define _AT(T,X)   ((T)(X))

#define UL(x)   _AC(x, UL)
#define ULL(x)  _AC(x, ULL)

#ifndef BIT
# define BIT(nr)          (1UL << (nr))
#endif

#ifndef BIT_ULL
# define BIT_ULL(nr)      (1ULL << (nr))
#endif

#ifndef GENMASK
# define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#endif
