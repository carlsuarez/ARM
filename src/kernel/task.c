#include "kernel/task.h"

struct task tasks[MAX_TASKS];
uint8_t current_task = 0;
uint8_t total_tasks = 0;
struct task *current = &tasks[0];

__attribute__((noreturn)) static void task_exit_trampoline(void)
{
    task_exit(0);
    while (1)
        ;
}

static void create_dummy(void)
{
    task_create(task_exit_trampoline);
    tasks[0].state = TERMINATED; // Mark dummy task as inactive
}

void task_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        for (int j = 0; j < STACK_SIZE; j++)
            tasks[i].stack[j] = 0xDEADBEEF;

        tasks[i].sp = NULL;
        tasks[i].state = TERMINATED;
    }
    create_dummy();
}

void task_create(void (*entry)(void))
{
    if (total_tasks >= MAX_TASKS)
        return;

    struct task *t = &tasks[total_tasks];

    // Set stack pointer to top of stack
    uint32_t *stack = (uint32_t *)(t->stack + STACK_SIZE);
    stack = (uint32_t *)((uint32_t)stack & ~0x3); // Word-align the stack

    uint32_t *original_sp = stack;

    *(--stack) = (uint32_t)entry; // LR_irq

    for (int i = 0; i <= 12; i++)
        *(--stack) = 0;

    *(--stack) = 0x10;                  // SPSR: user/system mode, IRQ enabled
    *(--stack) = (uint32_t)original_sp; // SP_orig
    *(--stack) = (uint32_t)task_exit;   // LR_orig

    t->sp = stack;
    t->state = READY;

    total_tasks++;
}

__attribute__((noreturn)) void task_exit(int32_t status)
{
    current->state = TERMINATED;
    uart_puts(uart0, "Task exiting with exit code: ");
    uart_puthex(uart0, status);
    uart_puts(uart0, "\n");
    cli(); // Enable interrupts
    while (1)
        ;
}

void scheduler(void)
{
    if (total_tasks == 0)
    {
        uart_puts(uart0, "No tasks available, entering infinite loop\n");
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
            uart_puts(uart0, "No task alive, entering infinite loop\n");
            while (1)
                ;
        }
        return; // Keep running the current task
    }

    if (current->state != TERMINATED)
        current->state = READY; // Mark previous task as ready if still alive

    current_task = next_task;
    current = &tasks[current_task];

    uart_puts(uart0, "Switching to task: ");
    uart_putc(uart0, '0' + current_task);
    uart_puts(uart0, "\n");

    current->state = RUNNING; // Mark new task as running
}
