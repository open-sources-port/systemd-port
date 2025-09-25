/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_INSTRUCTION_POINTER_H
#define _LINUX_INSTRUCTION_POINTER_H

#if defined(__x86_64__) || defined(__i386__)
    #include <x86/asm/linkage.h>
#elif defined(__aarch64__) || defined(__arm__)
    #include <arm64/asm/linkage.h>
#else
    #error "Unsupported architecture"
#endif

#define _RET_IP_		(unsigned long)__builtin_return_address(0)

#ifndef _THIS_IP_
#define _THIS_IP_  ({ __label__ __here; __here: (unsigned long)&&__here; })
#endif

#endif /* _LINUX_INSTRUCTION_POINTER_H */
