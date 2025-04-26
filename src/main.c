#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"
#include "task.h"

// Function prototypes
void task0(void);
void task1(void);

int kernel_main(void)
{
    cli();
    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);
    uart_puts(uart0, "Kernel main started\n");

    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;

    create_dummy(); // Initialize dummy task
    task_create(task0);
    task_create(task1);

    sei();

    TIMER1_START();

    while (1)
    {
        uart_puts(uart0, "Kernel main loop\n");
        for (volatile uint32_t i = 0; i < 1e9; i++)
            ;
    }
    return 0;
}

void task0(void)
{
    uart_puts(uart0, "Task 1 is running\n");
    for (volatile uint32_t i = 0; i < 1000000; i++)
        ;
    task_exit();
}
void task1(void)
{
    uart_puts(uart0, "Task 2 is running\n");
    for (volatile uint32_t i = 0; i < 1000000; i++)
        ;
    task_exit();
}