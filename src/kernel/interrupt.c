#include <stdint.h>
#include "kernel/interrupt.h"
#include "hw/pic.h"
#include "drivers/uart.h"
#include "hw/timer.h"
#include "kernel/task/task.h"

void irq_handler_c(void)
{
    uint32_t pic_status = pic->IRQ_STATUS;

    if (pic_status & PIC_UARTINT0)
    {
        // UART0 IRQ
        uint32_t uart_mis = uart0->mis;
        if (uart_mis & UART_MIS_RXMIS)
        {
            char c = uart0->dr;
            uart_putc(uart0, c);
            uart_putc(uart0, '\n');
        }
        uart0->icr = 0x03FF;
    }

    if (pic_status & PIC_TIMERINT1)
    {
        // Timer IRQ
        timer1->intclr = 0x1;

        scheduler();
    }

    if (pic_status & PIC_SOFTINT)
    {
        uart_puts(uart0, "Software Interrupt\n");
    }

    // IRQs will be re-enabled after we restore context and return
}