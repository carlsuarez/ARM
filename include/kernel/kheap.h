#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MIN_BLOCK_SIZE 16
#define BLOCK_DATA(b) ((void *)((block_t *)(b) + 1))
#define DATA_BLOCK(ptr) ((block_t *)(ptr) - 1)

typedef struct block
{
    size_t size;
    struct block *next;
    bool free;
} block_t;

static block_t *free_list = NULL;

void kheap_init(void *start, size_t size);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif
