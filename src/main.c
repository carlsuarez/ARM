#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"
#include "task.h"
#include "test.h"

// Function prototypes
void task1(void);
void task2(void);
extern char _irq_stack_top;

int kernel_main(void)
{
    sei();
    sef();

    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);
    uart_puts(uart0, "Kernel main started\n");

    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;

    task_init();
    task_create(task1);
    task_create(task2);

    uart_puts(uart0, "Task 1 address: ");
    uart_puthex(uart0, (uint32_t)task1);
    uart_putc(uart0, '\n');
    uart_puts(uart0, "Task 2 address: ");
    uart_puthex(uart0, (uint32_t)task2);
    uart_putc(uart0, '\n');
    uart_puts(uart0, "Task exit address: ");
    uart_puthex(uart0, (uint32_t)task_exit);
    uart_putc(uart0, '\n');
    uart_puts(uart0, "IRQ stack address: ");
    uart_puthex(uart0, (uint32_t)&_irq_stack_top);
    uart_putc(uart0, '\n');

    clf();
    cli();

    TIMER1_START();

    while (1)
        ;
}

void task1(void)
{
    uart_puts(uart0, "Task 1\n");
    dummy();
    task_exit();
}

void task2(void)
{
    uart_puts(uart0, "Task 2 is running\n");
    task_exit();
}