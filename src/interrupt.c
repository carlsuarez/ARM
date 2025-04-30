#include <stdint.h>
#include "interrupt.h"
#include "hw/pic.h"
#include "uart.h"
#include "hw/timer.h"
#include "task.h"

void irq_handler_c(void)
{
    uart_puts(uart0, "IRQ handler called\n");

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

        uart_puts(uart0, "Timer interrupt - running scheduler\n");

        scheduler();
    }

    if (pic_status & PIC_SOFTINT)
    {
        uart_puts(uart0, "Software Interrupt\n");
    }

    uart_puts(uart0, "New SP (IRQ Handler): ");
    uart_puthex(uart0, (uint32_t)current->sp);
    uart_putc(uart0, '\n');
    // IRQs will be re-enabled after we restore context and return
}