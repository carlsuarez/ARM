#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#include <defs.h>
#include <kernel/lib/malloc.h>
#include <kernel/arch/arm/mmu.h>
#include <kernel/lib/printk.h>

#define ALLOC_4K 1
#define ALLOC_1K 2
#define ALLOC_16K 3

typedef struct
{
    uint32_t *bitmap;
    size_t num_pages;
    size_t page_size;
    uintptr_t base_addr;
} PageAllocator;

void init_page_allocator(uint8_t n, size_t num_pages, size_t page_size, uintptr_t base_addr);
void *alloc_page(uint8_t n);
void free_page(uint8_t n, void *addr);

#endif
