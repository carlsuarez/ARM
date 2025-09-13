#include <kernel/lib/page_alloc.h>

static PageAllocator alloc_16k;
static PageAllocator alloc_4k;
static PageAllocator alloc_1k;

static inline int64_t find_first_zero(PageAllocator *alloc)
{
    size_t num_words = (alloc->num_pages + 31) / 32;
    for (size_t i = 0; i < num_words; i++)
    {
        if (alloc->bitmap[i] != 0xFFFFFFFF)
        {
            uint32_t inverted = ~alloc->bitmap[i];
            uint8_t bit = __builtin_ctz(inverted);
            alloc->bitmap[i] |= (1U << bit);
            return i * 32 + bit;
        }
    }
    return -1;
}

#define get_page_allocator(n)                                   \
    ((n) == ALLOC_4K ? &alloc_4k : (n) == ALLOC_1K ? &alloc_1k  \
                               : (n) == ALLOC_16K  ? &alloc_16k \
                                                   : NULL)

void init_page_allocator(uint8_t n, size_t num_pages, size_t page_size, uintptr_t base_addr)
{
    PageAllocator *alloc = get_page_allocator(n);

    if (!alloc)
        return;

    alloc->num_pages = num_pages;
    alloc->page_size = page_size;
    alloc->base_addr = base_addr;

    size_t num_words = (num_pages + 31) / 32;
    alloc->bitmap = kmalloc(num_words * sizeof(uint32_t));
    memset(alloc->bitmap, 0, num_words * sizeof(uint32_t));

    // Mask off unused bits in the last word
    size_t remaining_bits = num_pages % 32;
    if (remaining_bits != 0)
    {
        uint32_t mask = ~((1U << remaining_bits) - 1);
        alloc->bitmap[num_words - 1] |= mask;
    }
}

void *alloc_page(uint8_t n)
{
    PageAllocator *alloc = get_page_allocator(n);

    int64_t page = find_first_zero(alloc);
    if (page < 0)
        return NULL;

    void *addr = (void *)(alloc->base_addr + page * alloc->page_size);
    printk("allocating page @ %p (size: %u)\n", addr, alloc->page_size);
    return addr;
}

void free_page(uint8_t n, void *addr)
{
    if (!addr)
        return;

    PageAllocator *alloc = get_page_allocator(n);

    uintptr_t a = (uintptr_t)addr;
    if (a < alloc->base_addr || a >= (alloc->base_addr + alloc->num_pages * alloc->page_size))
        return; // Not in range

    memset(addr, 0, alloc->page_size);
    size_t page = (a - alloc->base_addr) / alloc->page_size;
    size_t word = page / 32;
    size_t bit = page % 32;

    alloc->bitmap[word] &= ~(1U << bit);
    printk("freed page @ %p\n", addr);
}