#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>
#include <stddef.h>

#define MEMORY_SIZE (128 * 1024) // 128KB
#define MIN_BLOCK_SIZE 32        // Smallest allocatable block
#define MAX_LEVELS (32 - __builtin_clz(MEMORY_SIZE / MIN_BLOCK_SIZE))

extern uint8_t _buddy_pool_start;

struct free_block
{
    struct free_block *next;
};

// One linked list for each order
static struct free_block *free_lists[MAX_LEVELS];

void buddy_init();
void *buddy_alloc(size_t size);
void buddy_free(void *ptr, size_t size);

#endif
