#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#include "defs.h"
#include "libc/kernel/malloc.h"
#include "arch/arm/mmu.h"
#include "libc/kernel/printk.h"

#define MEM_SIZE ((uint32_t)&_free_pages_end - (uint32_t)&_free_pages_start)
#define PAGE_SIZE 4096
#define NUM_PAGES (MEM_SIZE / PAGE_SIZE)

void init_page_alloc(void);

/**
 * @brief Allocates a free memory page and returns its address.
 *
 * This function searches for the first available (zeroed) page using
 * find_first_zero(). If a free page is found, it retrieves the corresponding
 * entry from the coarse page table (_coarse_pt0_start), masks the address to
 * obtain the page-aligned address, and returns it as a pointer.
 *
 * @return Pointer to the allocated page on success, or NULL if no free page is available.
 * @note This function returns an identity mapped virtual address (equivalent to physical address).
 */
void *alloc_page(void);

void free_page(void *ptr);

#endif
