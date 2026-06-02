#ifndef _KERNEL_ASSERT_H
#define _KERNEL_ASSERT_H

#include <kernel/panic.h>

#define kassert(expr) \
    do { \
        if (!(expr)) { \
            panic("assertion failed: " #expr); \
        } \
    } while (0)

#endif
