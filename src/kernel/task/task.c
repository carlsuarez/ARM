#include "kernel/task/task.h"

task_t *tasks = NULL;
static size_t current_task = 0;
static size_t total_tasks = 0;
task_t *current = NULL;

static slab_cache_t *cache = NULL;

__attribute__((noreturn)) static void task_exit_trampoline(void)
{
    task_exit(0);
    while (1)
        ;
}

static void create_dummy(void)
{
    task_create((uintptr_t)task_exit_trampoline);
    tasks->state = TERMINATED; // Mark dummy task as inactive
}

void task_init(void)
{
    cache = create_slab_cache(sizeof(task_t));
    create_dummy();
}

task_t *task_create(uintptr_t entry)
{
    if (total_tasks >= MAX_TASKS)
        return NULL;

    task_t *new_task = slab_alloc(cache);

    task_t **p = &tasks;
    while (*p)
        p = &(*p)->next;
    *p = new_task;
    new_task->next = NULL;

    // Allocate page for stack
    uint32_t *stack = alloc_page();
    new_task->stack_base = (uintptr_t)stack;
    printk("Stack bottom: 0x%x", stack);
    printk("\n");

    stack += PAGE_SIZE / sizeof(uint32_t); // Move to end of stack

    printk("Stack top: 0x%x", stack);
    printk("\n");

    uint32_t *original_sp = stack;

    *(--stack) = (uint32_t)entry; // LR_irq

    for (int i = 0; i <= 12; i++)
        *(--stack) = 0;

    *(--stack) = 0x10;                  // SPSR: user/system mode, IRQ enabled
    *(--stack) = (uint32_t)original_sp; // SP_orig
    *(--stack) = (uint32_t)task_exit;   // LR_orig

    new_task->sp = stack;
    new_task->state = READY;

    new_task->pid = total_tasks++;

    return new_task;
}

__attribute__((noreturn)) void task_exit(int32_t status)
{
    current->state = TERMINATED;
    set_l1_entry(current->coarse_pt, 0);

    uint32_t *pt = (uint32_t *)current->coarse_pt;
    for (size_t i = 0; i < current->num_pages; i++)
    {
        uintptr_t addr = current->coarse_pt + i * 0x1000;
        free_page(addr);
        pt[addr >> 12] = 0;
    }

    free_page((void *)current->coarse_pt);
    free_page((void *)current->stack_base);
    slab_free(cache, current);
    printk("Task exiting with exit code: %d\n", status);

    cli(); // Enable interrupts
    while (1)
        ;
}

void scheduler(void)
{
    if (total_tasks == 0)
        while (1)
            ;

    task_t *next = current;
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

    // Change active user L2
    set_l1_entry(current->coarse_pt, 0); // Clear current entry
    set_l1_entry(next->va_base, COARSE_ENTRY(next->coarse_pt, DOMAIN_USER));

    current = next;
    current->state = RUNNING;

    printk("Switching to task %s\n", current->name);
}