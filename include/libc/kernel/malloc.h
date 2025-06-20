#ifndef KERNEL_MALLOC_H
#define KERNEL_MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include "libc/common/memory.h"

#define MEMORY_SIZE 0x20000 // 128 KB
#define MIN_BLOCK_SIZE 32   // Smallest allocatable block
#define MAX_LEVELS (32 - __builtin_clz(MEMORY_SIZE / MIN_BLOCK_SIZE))

struct free_block
{
    struct free_block *next;
};

void kheap_init(uintptr_t start);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif
