#include "task.h"

static struct task tasks[MAX_TASKS];
static uint8_t current_task = 0;
static uint8_t total_tasks = 0;

void create_dummy(void)
{
    tasks[0].sp = NULL;
    tasks[0].active = 0;
}

void task_create(void (*entry)(void))
{
    uart_puts(uart0, "Creating task...\n");
    if (total_tasks >= MAX_TASKS)
    {
        uart_puts(uart0, "Max tasks reached: ");
        uart_puthex(uart0, total_tasks);
        uart_putc(uart0, '\n');
        return;
    }

    struct task *t = &tasks[total_tasks];

    uint32_t *sp = &(t->stack[STACK_SIZE]);

    *(--sp) = (uint32_t)task_exit_trampoline; // LR
    *(--sp) = (uint32_t)entry;                // PC
    *(--sp) = 0;                              // r11
    *(--sp) = 0;                              // r10
    *(--sp) = 0;                              // r9
    *(--sp) = 0;                              // r8
    *(--sp) = 0;                              // r7
    *(--sp) = 0;                              // r6
    *(--sp) = 0;                              // r5
    *(--sp) = 0;                              // r4

    t->sp = sp;
    t->active = 1;

    total_tasks++;

    uart_puts(uart0, "Total tasks: ");
    uart_puthex(uart0, total_tasks);
    uart_putc(uart0, '\n');
}

void task_exit()
{
    uart_puts(uart0, "Task exiting...\n");
    tasks[current_task].active = 0;
    scheduler();
}

void scheduler(void)
{
    uart_puts(uart0, "Scheduler called\n");
    uart_puts(uart0, "Current task: ");
    uart_puthex(uart0, current_task);
    uart_putc(uart0, '\n');

    if (total_tasks == 0)
    {
        while (1)
            ; // No tasks at all — spin or panic
    }

    uint8_t next_task = current_task;
    uint8_t found = 0;

    // Search for the next active task (excluding current_task at first)
    for (uint8_t i = 1; i < total_tasks; i++)
    {
        next_task = (current_task + i) % total_tasks;
        if (tasks[next_task].active)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        // No other active tasks, but maybe current_task is still alive
        if (tasks[current_task].active)
        {
            // Stick with current
            return;
        }
        else
        {
            // No tasks are alive — deadlock or panic
            uart_puts(uart0, "No active tasks remaining.\n");
            while (1)
                ;
        }
    }

    uint8_t old_task = current_task;
    current_task = next_task;
    uart_puts(uart0, "Switching from task: ");
    uart_puthex(uart0, old_task);
    uart_putc(uart0, '\n');
    uart_puts(uart0, "Switching to task: ");
    uart_puthex(uart0, current_task);
    uart_putc(uart0, '\n');

    sei();
    context_switch(&tasks[old_task].sp, &tasks[current_task].sp);
}
