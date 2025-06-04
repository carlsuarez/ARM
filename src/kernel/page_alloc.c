#include "kernel/page_alloc.h"

static uint32_t *bitmap;

void init_page_alloc(void)
{
    size_t num_words = (NUM_PAGES + 31) / 32;
    bitmap = kmalloc(num_words * sizeof(uint32_t));
    memset(bitmap, 0, num_words * sizeof(uint32_t));

    // Mask off unused bits in last word, if any
    size_t remaining_bits = NUM_PAGES % 32;
    if (remaining_bits != 0)
    {
        uint32_t mask = ~((1U << remaining_bits) - 1); // 1s in the unused bits
        bitmap[num_words - 1] |= mask;                 // Mark them as used
    }
}

static inline int64_t find_first_zero(void)
{
    for (size_t i = 0; i < NUM_PAGES / 32; i++)
    {
        if (bitmap[i] != 0xFFFFFFFF)
        {
            uint32_t inverted = ~bitmap[i];
            uint8_t bit = __builtin_ctz(inverted);
            // Mark bit as allocated
            bitmap[i] |= (1U << bit);
            return i * 32 + bit;
        }
    }
    return -1; // No free pages
}

void *alloc_page()
{
    int64_t page = find_first_zero();
    if (page < 0)
        return NULL;

    uint32_t *coarse_pt0 = (uint32_t *)&_coarse_pt0_start;

    return (void *)(coarse_pt0[page] & 0xFFFFF000);
}

void free_page(void *addr)
{
    if (addr == NULL)
        return;

    if (((uint32_t)addr >> 20) != ((uint32_t)FREE_PAGES_VA_BASE >> 20)) // Not an address in the virtual space of the coarse_pt0 table
        return;

    memset(addr, 0, PAGE_SIZE); // Clear the page

    size_t page = ((uintptr_t)addr) >> 12;

    size_t word = page / 32;
    size_t bit = page % 32;

    bitmap[word] &= ~(1U << bit); // Mark page as free (0)
}
