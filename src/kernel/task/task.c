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
    task_create("dummy", (uintptr_t)task_exit_trampoline);
    tasks->state = TERMINATED; // Mark dummy task as inactive
}

void task_init(void)
{
    cache = create_slab_cache(sizeof(task_t));
    create_dummy();
}

void task_create(char *name, uintptr_t entry)
{
    if (total_tasks >= MAX_TASKS)
        return;

    task_t *new_task = slab_alloc(cache);

    task_t **p = &tasks;
    while (*p)
        p = &(*p)->next;
    *p = new_task;
    new_task->next = NULL;

    // Allocate page for stack
    uint32_t *stack = alloc_page();
    new_task->stack_base = (uintptr_t)stack;
    printk("Stack bottom: 0x%x\n", stack);

    stack += PAGE_SIZE / sizeof(uint32_t); // Move to end of stack

    printk("Stack top: 0x%x\n", stack);

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
}

__attribute__((noreturn)) void task_exit(int32_t status)
{
    current->state = TERMINATED;
    printk("Task exiting with exit code: %d\n", status);

    cli(); // Enable interrupts
    while (1)
        ;
}

void scheduler(void)
{
    if (total_tasks == 0)
    {
        printk("No tasks available, entering infinite loop\n");
        while (1)
            ;
    }

    uint8_t next_task = current_task;
    uint8_t found = 0;

    for (uint8_t i = 0; i < total_tasks; i++)
    {
        next_task = (next_task + 1) % total_tasks;
        if (tasks[next_task].state == READY)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        if (current->state == TERMINATED)
        {
            printk("No task alive, entering infinite loop\n");
            while (1)
                ;
        }
        return; // Keep running the current task
    }

    if (current->state != TERMINATED)
        current->state = READY; // Mark previous task as ready if still alive

    current_task = next_task;
    current = &tasks[current_task];

    printk("Switching to task: %u\n", current_task);

    current->state = RUNNING; // Mark new task as running
}
