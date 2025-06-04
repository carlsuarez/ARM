#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#include "defs.h"
#include "kernel/kheap.h"
#include "arch/arm/mmu.h"

#define MEM_SIZE ((uint32_t)&_free_pages_end - (uint32_t)&_free_pages_start)
#define PAGE_SIZE 4096
#define NUM_PAGES (MEM_SIZE / PAGE_SIZE)

void init_page_alloc(void);
void *alloc_page(void);
void free_page(void *ptr);

#endif
