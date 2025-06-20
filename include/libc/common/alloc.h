#ifndef COMMON_ALLOC_H
#define COMMON_ALLOC_H

#ifdef __KERNEL__
#include "libc/kernel/malloc.h"

static inline void *lib_malloc(size_t size)
{
    return kmalloc(size);
}

static inline void lib_free(void *ptr)
{
    kfree(ptr);
}
#else
#include "libc/user/malloc.h"

static inline void *lib_malloc(size_t size)
{
    return malloc(size);
}

static inline void lib_free(void *ptr)
{
    free(ptr);
}
#endif

#endif