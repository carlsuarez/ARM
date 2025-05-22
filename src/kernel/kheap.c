#include "kernel/kheap.h"

void kheap_init(void *start, size_t size)
{
    free_list = (block_t *)start;
    free_list->size = size - sizeof(block_t);
    free_list->next = NULL;
    free_list->free = true;
}

void *kmalloc(size_t size)
{
    block_t *curr = free_list;

    while (curr != NULL)
    {
        if (curr->free && size <= curr->size)
        {

            // Don't split if new block would be smaller than MIN_BLOCK_SIZE bytes
            if (curr->size >= size + sizeof(block_t) + MIN_BLOCK_SIZE)
            {
                block_t *new_block = (block_t *)((uintptr_t)BLOCK_DATA(curr) + curr->size);
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->free = true;

                curr->size = size;
                curr->next = new_block;
            }

            curr->free = false;
            return BLOCK_DATA(curr);
        }

        curr = curr->next;
    }

    return NULL;
}

static void coalesce(void)
{
    block_t *curr = free_list;

    while (curr != NULL && curr->next != NULL)
    {
        if (curr->free && curr->next->free)
        {
            block_t *next = curr->next;

            // Merge curr and next
            curr->size += sizeof(block_t) + next->size;
            curr->next = next->next;
            // Don't advance curr — check again in case multiple free blocks in a row
        }
        else
        {
            curr = curr->next;
        }
    }
}

void kfree(void *ptr)
{
    if (!ptr)
        return;

    block_t *block = DATA_BLOCK(ptr);

    block->free = true;

    coalesce();
}

void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr)
        return kmalloc(new_size);

    if (new_size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    block_t *curr = DATA_BLOCK(ptr);
    size_t current_size = curr->size;

    if (new_size <= current_size)
        return ptr; // Shrinking, no need to move

    // Try to expand into the next free block
    block_t *next = curr->next;
    if (next && next->free && (current_size + sizeof(block_t) + next->size) >= new_size)
    {
        // Merge current and next
        curr->size += sizeof(block_t) + next->size;
        curr->next = next->next;

        // Optionally split if too big
        size_t remaining = curr->size - new_size;
        if (remaining > sizeof(block_t) + MIN_BLOCK_SIZE) // minimal alloc unit
        {
            block_t *split = (block_t *)((uintptr_t)BLOCK_DATA(curr) + new_size);
            split->size = remaining - sizeof(block_t);
            split->free = true;
            split->next = curr->next;

            curr->size = new_size;
            curr->next = split;
        }

        return BLOCK_DATA(curr);
    }

    // Fallback: allocate and copy
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr)
        return NULL;

    for (size_t i = 0; i < current_size; i++)
        ((uint8_t *)new_ptr)[i] = ((uint8_t *)ptr)[i];

    kfree(ptr);
    return new_ptr;
}
