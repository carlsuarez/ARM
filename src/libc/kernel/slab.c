#include "libc/kernel/slab.h"

slab_cache_t *create_slab_cache(size_t object_size)
{
    slab_cache_t *cache = kmalloc(sizeof(slab_cache_t));
    cache->object_size = object_size; // Use aligned size
    cache->slabs = NULL;
    return cache;
}

void destroy_slab_cache(slab_cache_t *cache)
{
    slab_header_t *slab = cache->slabs;

    while (slab)
    {
        slab_header_t *next = slab->next;
        free_page(slab); // Free the entire slab page
        slab = next;
    }

    kfree(cache); // Free the cache metadata
}

static void slab_add_slab(slab_cache_t *cache)
{
    void *slab_memory = alloc_page();

    // Ensure slab header is properly aligned
    slab_header_t *header = (slab_header_t *)slab_memory;
    header->next = cache->slabs;
    header->used_count = 0;
    cache->slabs = header;

    // Calculate available space after header (with alignment)
    uintptr_t start = (uintptr_t)(header + 1);
    start = (start + (cache->object_size - 1)) & ~(cache->object_size - 1);

    size_t remaining_space = PAGE_SIZE - (start - (uintptr_t)slab_memory);
    uint32_t num_objects = remaining_space / cache->object_size;

    // Build free list
    void *prev = NULL;
    for (int i = num_objects - 1; i >= 0; i--)
    {
        void *obj = (void *)(start + i * cache->object_size);
        *(void **)obj = prev;
        prev = obj;
    }
    header->free_list = prev;
}

void *slab_alloc(slab_cache_t *cache)
{
    slab_header_t *slab = cache->slabs;

    while (slab)
    {
        if (slab->free_list)
        {
            void *obj = slab->free_list;
            slab->free_list = *(void **)obj;
            slab->used_count++;
            return obj;
        }
        slab = slab->next;
    }

    // No free object found, create a new slab
    slab_add_slab(cache);
    return slab_alloc(cache); // try again
}

void slab_free(slab_cache_t *cache, void *obj)
{
    slab_header_t *slab = cache->slabs;

    while (slab)
    {
        uintptr_t start = (uintptr_t)(slab + 1);
        uintptr_t end = (uintptr_t)slab + PAGE_SIZE;

        if ((uintptr_t)obj >= start && (uintptr_t)obj < end)
        {
            *(void **)obj = slab->free_list;
            slab->free_list = obj;
            slab->used_count--;
            return;
        }

        slab = slab->next;
    }
}