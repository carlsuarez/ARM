#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"
#include "task.h"
#include "test.h"
#include "syscall.h"

// Function prototypes
void task1(void);
void task2(void);
void task3(void);

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
    task_create(task3);

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
    syscall(SYS_EXIT, 0, 0, 0, 0);
}

void task2(void)
{
    uart_puts(uart0, "Task 2 is running\n");
    syscall(SYS_EXIT, 0, 0, 0, 0);
}

void task3(void)
{
    uart_puts(uart0, "Task 3 is running\n");
    for (volatile uint32_t i = 0; i < 1e9; i++)
        ;
    syscall(SYS_EXIT, 0, 0, 0, 0);
}