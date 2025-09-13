#include <kernel/core/task/elf/so_loader.h>

static so_entry_t *loaded_so_list;

static inline bool in_global_list(const char *name, so_entry_t **found)
{
    so_entry_t *cur = loaded_so_list;
    while (cur)
    {
        if (strcmp(name, cur->name) == 0)
        {
            *found = cur;
            return true;
        }
        cur = cur->next;
    }

    return false;
}

static inline void add_to_global_list(so_entry_t *node)
{
    node->next = loaded_so_list;
    loaded_so_list = node;
}

static inline bool in_task_list(const char *name, so_entry_task_t *list)
{
    so_entry_task_t *cur = list;
    while (cur)
    {
        if (strcmp(name, cur->so->name) == 0)
        {
            return true;
        }
        cur = cur->next;
    }

    return false;
}

static void add_to_task_list(so_entry_t *node, struct PCB *task)
{
    so_entry_task_t *new_entry = slab_alloc(so_entry_task_cache);
    if (!new_entry)
        return;

    new_entry->so = node;
    new_entry->base_va = task->elf_info.next_so_base;
    new_entry->next = task->shared_objs;
    task->shared_objs = new_entry;
}

static void map_so(so_entry_t *so, struct PCB *task)
{
    uintptr_t base_va = task->elf_info.next_so_base;
    uint32_t *l1 = task->pt;
    for (size_t i = 0; i < so->num_pages; i++)
    {
        uintptr_t page_phys = so->pages[i].page_phys;
        uintptr_t va = base_va + so->pages[i].offset;

        uint32_t l1_entry = l1[L1_INDEX(va)];
        if (is_valid_l1_coarse_entry(l1_entry))
            map_page(COARSE_BASE(l1_entry), va, page_phys, AP(AP_USER_RW));
    }

    task->elf_info.next_so_base += so->num_pages * SMALL_PAGE_SIZE;
}

int8_t load_shared_object(const char *name, struct PCB *task)
{
    printk("Loading shared object %s\n", name);
    if (!so_entry_cache)
    {
        // Create a cache if there isn't one
        so_entry_cache = create_slab_cache(sizeof(so_entry_t));
    }

    if (!so_entry_cache)
        return -1;

    if (!so_entry_task_cache)
        so_entry_task_cache = create_slab_cache(sizeof(so_entry_task_t));

    if (!so_entry_task_cache)
        return -1;

    so_entry_t *found;
    if (in_global_list(name, &found))
    {
        printk("SO already loaded globally\n");
        /* Loaded globally (doesn't need to be loaded)*/
        if (in_task_list(name, task->shared_objs))
        {
            printk("SO already loaded in task\n");
            return 0;
        }

        found->ref_count++;
        add_to_task_list(found, task);
        map_so(found, task);
        return 0;
    }

    /* Not loaded globally (brand new SO) */
    printk("Brand new SO\n");
    so_entry_t *new_so = slab_alloc(so_entry_cache);
    new_so->name = name;
    new_so->next = NULL;
    new_so->ref_count = 0;

    add_to_global_list(new_so);
    add_to_task_list(new_so, task);

    if (elf_load_internal(name, task, true, NULL, new_so) < 0)
        return -1;

    return 0;
}

uintptr_t resolve_symbol(const char *sym_name, so_entry_task_t *list)
{
    so_entry_task_t *node = list;
    while (node)
    {
        so_entry_t *cur = node->so;
        printk("cur->name: %s\n", cur->name);
        printk("cur->strtab: %p\n", cur->strtab);
        printk("cur->symtab: %p\n", cur->symtab);
        printk("cur->hash.bucket: %p\n", cur->hash.bucket);
        printk("cur->hash.chain: %p\n", cur->hash.chain);

        Elf32_Word nbucket = cur->hash.nbucket;
        Elf32_Word *bucket = cur->hash.bucket;
        Elf32_Word *chain = cur->hash.chain;
        const char *strtab = cur->strtab;
        Elf32_Sym *symtab = cur->symtab;

        unsigned long hash = elf_hash((unsigned char *)sym_name);
        size_t index = bucket[hash % cur->hash.nbucket];

        while (index != STN_UNDEF)
        {
            // Compare the names
            /*
                - Access symtab entry via index
                - Access strtab entry via st_name
            */

            char *name = strtab + symtab[index].st_name;
            printk("Comparing symbols %s and %s\n", name, sym_name);
            if (strcmp(name, sym_name) == 0)
            {
                return symtab[index].st_value + node->base_va; // found it
            }

            // Follow the chain
            index = chain[index];
        }

        // Go to the next shared object
        node = node->next;
    }

    // Not found
    return 0;
}

void unload_shared_objects(struct PCB *task)
{
    so_entry_task_t *node = task->shared_objs;
    so_entry_task_t *next;

    while (node)
    {
        next = node->next; // save next node first
        so_entry_t *cur = node->so;

        if (cur)
        {
            cur->ref_count--;
            if (cur->ref_count == 0)
            {
                kfree(cur->strtab);
                kfree(cur->symtab);
                kfree(cur->hash.bucket);
                kfree(cur->hash.chain);
                kfree(cur->pages);
                fat32_close(cur->fd);
                slab_free(so_entry_cache, cur);
            }
        }

        slab_free(so_entry_task_cache, node); // free the wrapper
        node = next;
    }

    task->shared_objs = NULL;
}
