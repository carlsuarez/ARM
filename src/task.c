#include "task.h"

struct task tasks[MAX_TASKS];
uint8_t current_task = 0;
uint8_t total_tasks = 0;
struct task *current = &tasks[0];

static void create_dummy(void)
{
    task_create(task_exit);
    tasks[0].active = 0; // Mark dummy task as inactive
}

void task_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        for (int j = 0; j < STACK_SIZE; j++)
            tasks[i].stack[j] = 0xDEADBEEF;

        tasks[i].sp = NULL;
        tasks[i].active = 0;
    }
    create_dummy();
}

void task_create(void (*entry)(void))
{
    uart_puts(uart0, "Creating task...\n");
    if (total_tasks >= MAX_TASKS)
        return;

    struct task *t = &tasks[total_tasks];

    // Set stack pointer to top of stack
    uint32_t *stack = (uint32_t *)(t->stack + STACK_SIZE);
    stack = (uint32_t *)((uint32_t)stack & ~0x3); // Word-align the stack

    // Fake context: emulate saved registers

    /* Remember this as the "original" SP value */
    uint32_t *original_sp = stack;

    /* Create the simulated IRQ stack frame */

    /* LR_irq (return address for IRQ) */
    *(--stack) = (uint32_t)entry; /* LR_irq: task entry point */

    /* Initialize r0-r12 (13 registers) */
    for (int i = 0; i <= 12; i++)
    {
        *(--stack) = 0; /* r1-r12 = 0 */
    }

    *(--stack) = 0x1F; /* SPSR: target mode with IRQs enabled */

    /* Original mode LR and SP */
    *(--stack) = (uint32_t)original_sp; /* SP_orig: original stack pointer */

    /* Simulate the return address for the task */
    *(--stack) = (uint32_t)task_exit; /* LR_orig */

    t->sp = stack; /* Set task stack pointer */
    t->active = 1; /* Mark task as active */

    total_tasks++;
}

void task_exit()
{
    current->active = 0;
    uart_puts(uart0, "Task exiting...\n");
    while (1)
        ;
}

void scheduler(void)
{
    uart_puts(uart0, "Scheduler called\n");
    uart_puts(uart0, "--- Preempted Task Stack Contents ---\n");
    uart_puts(uart0, "Stack pointer: ");
    uart_puthex(uart0, (uint32_t)current->sp);
    uart_putc(uart0, '\n');
    for (int i = 0; i < 16; i++)
    {
        uart_puthex(uart0, i);
        uart_puts(uart0, ": ");
        uart_puthex(uart0, current->sp[i]);
        uart_putc(uart0, '\n');
    }
    uart_puts(uart0, "Active? ");
    uart_puts(uart0, current->active ? "Yes\n" : "No\n");
    uart_puts(uart0, "------------------------------\n");

    if (total_tasks == 0)
    {
        // No tasks, panic
        uart_puts(uart0, "No tasks available, entering infinite loop\n");
        while (1)
            ;
    }

    // Find next active task
    uint8_t next_task = current_task;
    uint8_t found = 0;

    for (uint8_t i = 0; i < total_tasks; i++)
    {
        next_task = (next_task + 1) % total_tasks;
        if (tasks[next_task].active)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        if (!current->active)
        {
            // No task alive
            uart_puts(uart0, "No task alive, entering infinite loop\n");
            while (1)
                ;
        }
        return; // Continue running current task
    }

    // Switch to new task
    current_task = next_task;
    current = &tasks[current_task];

    uart_puts(uart0, "Switching to task: ");
    uart_putc(uart0, '0' + current_task);
    uart_putc(uart0, '\n');
    uart_puts(uart0, "--- New Task Stack Contents ---\n");
    uart_puts(uart0, "Stack pointer: ");
    uart_puthex(uart0, (uint32_t)current->sp);
    uart_putc(uart0, '\n');
    for (int i = 0; i < 16; i++)
    {
        uart_puthex(uart0, i);
        uart_puts(uart0, ": ");
        uart_puthex(uart0, current->sp[i]);
        uart_putc(uart0, '\n');
    }
    uart_puts(uart0, "Active? ");
    uart_puts(uart0, current->active ? "Yes\n" : "No\n");
    uart_puts(uart0, "------------------------------\n");
}