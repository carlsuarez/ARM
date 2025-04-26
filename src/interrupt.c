#include <stdint.h>
#include "interrupt.h"
#include "hw/pic.h"
#include "uart.h"
#include "hw/timer.h"
#include "task.h"

void irq_handler_c(void)
{
    uint32_t pic_status = pic->IRQ_STATUS; // Read the interrupt status from the PIC

    if (pic_status & PIC_UARTINT0)
    {
        // Handle UART0 interrupt
        uint32_t uart_mis = uart0->mis; // Read the masked interrupt status from UART0

        // Recieve interrupt
        if (uart_mis & UART_MIS_RXMIS)
        {
            // Read the received character
            char c = uart0->dr;
            uart_putc(uart0, c); // Echo the character back
            uart_putc(uart0, '\n');
        }
        uart0->icr = 0x03FF; // Clear the UART0 interrupt
    }

    if (pic_status & PIC_TIMERINT1)
    {
        uart_puts(uart0, "Timer1 Interrupt\n");
        timer1->intclr = 0x1; // Clear the Timer1 interrupt

        scheduler();
    }

    if (pic_status & PIC_SOFTINT)
    {
        // Handle software interrupt
        uart_puts(uart0, "Software Interrupt\n");
    }
}