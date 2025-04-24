#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"

int kernel_main(void)
{
    cli();
    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);
    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;
    sei();
    TIMER1_START();

    uart_puts(uart0, "Hello world\n"); // Send a message to UART0
    while (1)
        ;
    return 0;
}