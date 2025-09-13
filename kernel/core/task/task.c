#include <kernel/core/task/task.h>

struct PCB *tasks = NULL;
static size_t current_task = 0;
static size_t total_tasks = 0;
struct PCB *current = NULL;

static slab_cache_t *pcb_cache = NULL;
static l1_free_node_t *l1_free_list = NULL;
static slab_cache_t *l1_free_cache = NULL;

void task_init(void)
{
    pcb_cache = create_slab_cache(sizeof(struct PCB));
    l1_free_cache = create_slab_cache(sizeof(l1_free_node_t));
}

// Add an L1 table to the deferred free list
static void add_to_l1_free_list(uint32_t *l1)
{
    if (!l1)
        return;

    if (!l1_free_cache)
        l1_free_cache = create_slab_cache(sizeof(l1_free_node_t));

    l1_free_node_t *node = slab_alloc(l1_free_cache);
    node->l1 = l1;
    node->next = l1_free_list;
    l1_free_list = node;
}

static void free_pending_l1_tables(void)
{
    l1_free_node_t *node = l1_free_list;

    while (node)
    {
        // Free the physical memory of the L1 table
        free_page(ALLOC_16K, node->l1);

        l1_free_node_t *next = node->next;
        slab_free(l1_free_cache, node);
        node = next;
    }

    l1_free_list = NULL;
}

static void map_stack(uint32_t *pt)
{
    printk("Allocating pages and mapping stack...\n");
    const size_t num_pages = TASK_STACK_SIZE / SMALL_PAGE_SIZE;
    uint32_t *coarse_pt;
    for (size_t i = 0; i < num_pages; i++)
    {
        uintptr_t va = TASK_STACK_BASE + i * SMALL_PAGE_SIZE; // Compute virtual address

        if (i % NUM_COARSE_ENTRIES == 0) // Check if new coarse page table is needed
        {
            // Allocate coarse page table and set l1 entry
            coarse_pt = (uint32_t *)alloc_page(ALLOC_1K);
            pt[L1_INDEX(va)] = COARSE_ENTRY((uintptr_t)coarse_pt, DOMAIN_USER);
        }

        uintptr_t page_phys = (uintptr_t)alloc_page(ALLOC_4K);                // Allocate new page
        coarse_pt[i] = L2_PAGE_ENTRY(page_phys, AP(AP_USER_RW), C_WT, B_BUF); // Set coarse entry
    }
    printk("Done\n");
}

int8_t task_create(const char *path, const char *name)
{
    printk("Creating task %s\n", name);
    if (total_tasks >= MAX_TASKS)
        return -1; // Cannot create any more tasks

    struct PCB *task = slab_alloc(pcb_cache);

    if (!task)
        return -1; // Failed to allocate slab

    // Add task struct to the end of the linked list
    struct PCB **p = &tasks;
    while (*p)
        p = &(*p)->next;
    *p = task;
    task->next = NULL;

    // Copy task name into the struct
    strncpy(task->name, name, 11);

    task->elf_info.base_va = TASK_TEXT_BASE;
    task->elf_info.next_so_base = TASK_SO_BASE;
    task->shared_objs = NULL;

    // Allocate L1 page table
    task->pt = (uint32_t *)alloc_page(ALLOC_16K);

    printk("task page table: %p\n", task->pt);

    // Initialize the task's page table
    init_page_table(task->pt);

    // Map the stack
    map_stack(task->pt);

    // Load the elf file
    uintptr_t entry = elf_load(path, task);

    if (!entry)
        return -1;

    task->sp = TASK_STACK_BASE + TASK_STACK_SIZE - 1024;
    printk("Stack top: %p", task->sp);
    printk("\n");

    task->context[0] = (uint32_t)entry; // LR (will become PC on return)
    for (int i = 1; i <= 13; i++)
    { // r0-r12
        task->context[i] = 0;
    }
    task->context[14] = 0x10;                // SPSR: user/system mode, IRQ enabled
    task->context[15] = (uint32_t)task->sp;  // Original SP
    task->context[16] = (uint32_t)task_exit; // Original LR

    task->state = READY;

    // TODO change this logic. Doesn't produce unique IDs
    task->pid = total_tasks++;

    printk("entry: %p\n", task->context[0]);
    printk("task->sp: %p\n", task->context[15]);
    printk("task_exit: %p\n", task->context[16]);
    return 0;
}

__attribute__((noreturn)) void task_exit(int32_t status)
{
    current->state = TERMINATED;

    printk("Task exiting with exit code: %d\n", status);

    // Free all parts that were dynamically allocated by elf_load
    kfree(current->elf_info.strtab);
    kfree(current->elf_info.symtab);
    kfree(current->elf_info.hash.bucket);
    kfree(current->elf_info.hash.chain);

    unload_shared_objects(current);

    for (size_t i = 0; i < NUM_L1_ENTRIES; i++)
    {
        uint32_t l1_entry = current->pt[i];
        if (!is_valid_l1_coarse_entry(l1_entry))
            continue;

        uint32_t *coarse_pt = COARSE_BASE(l1_entry);
        for (size_t j = 0; j < NUM_COARSE_ENTRIES; j++)
        {
            uint32_t coarse_entry = coarse_pt[j];
            if (!is_valid_l2_coarse_entry(coarse_entry))
                continue;

            free_page(ALLOC_4K, COARSE_PAGE_BASE(coarse_entry));
        }

        free_page(ALLOC_1K, coarse_pt);
    }

    printk("Freed all pages in l1 page table @ %p\n", current->pt);

    add_to_l1_free_list(current->pt);
    slab_free(pcb_cache, current);

    total_tasks--;
    cli(); // Enable interrupts
    while (1)
        ;
}

static inline void set_page_table(uint32_t *l1_table)
{
    uint32_t ttbr = (uint32_t)l1_table;

    /* Write TTBR0 */
    asm volatile(
        "mcr p15, 0, %0, c2, c0, 0\n" /* TTBR <- ttbr */
        :
        : "r"(ttbr)
        : "memory");

    /* Invalidate entire TLB (TLBIALL). Use r0=0 as operand on many cores. */
    asm volatile(
        "mov r0, #0\n"
        "mcr p15, 0, r0, c8, c7, 0\n" /* TLBIALL */
        :
        :
        : "r0", "memory");

    /* Invalidate instruction & data caches if you rely on them
       (optional; can be omitted if your system doesn't require it
       or if caches are coherent/managed elsewhere). */
    asm volatile(
        "mov r0, #0\n"
        "mcr p15, 0, r0, c7, c5, 0\n"  /* I-pcb_cache invalidate (IC ICIALL) */
        "mcr p15, 0, r0, c7, c10, 4\n" /* D-pcb_cache clean/invalidate (implementation-dependent) */
        :
        :
        : "r0", "memory");

    /* A small barrier: a few no-ops / dummy read to help ordering on older cores */
    asm volatile("nop\nnop\n" : : : "memory");
}

void scheduler(void)
{
    printk("Scheduler\n");

    if (total_tasks == 0)
    {
        printk("No tasks...\n");
        while (1)
            ;
    }

    struct PCB *next = current ? current : tasks;
    for (size_t i = 0; i < total_tasks; ++i)
    {
        next = next->next ? next->next : tasks;
        if (next->state == READY)
            break;
    }

    if (next == current || next->state != READY)
        return;

    if (current->state != TERMINATED)
        current->state = READY;

    current = next;
    current->state = RUNNING;

    printk("Switching to task %s\n", current->name);

    set_page_table(current->pt);

    free_pending_l1_tables();
}