#include <stdint.h>
#include "interrupt.h"
#include "hw/pic.h"
#include "uart.h"
#include "hw/timer.h"

void identify_and_clear_source(void)
{
    uint32_t pic_status = *PIC_IRQ_STATUS; // Read the interrupt status from the PIC

    uart_puts(uart0, "Identify and clear source\n");

    if (pic_status & PIC_UARTINT0)
    {
        uart0->icr = 0x03FF; // Clear the UART0 interrupt
    }

    if (pic_status & PIC_UARTINT1)
    {
        // Clear the UART1 interrupt
        uart1->icr = 0x03FF; // Clear the UART1 interrupt
    }

    if (pic_status & PIC_TIMERINT0)
    {
        // Clear the Timer0 interrupt
        timer0->intclr = 0x1; // Clear the Timer0 interrupt
    }

    if (pic_status & PIC_TIMERINT1)
    {
        // Clear the Timer1 interrupt
        timer1->intclr = 0x1; // Clear the Timer1 interrupt
    }

    if (pic_status & PIC_TIMERINT2)
    {
        // Clear the Timer2 interrupt
        timer2->intclr = 0x1; // Clear the Timer2 interrupt
    }
}

void irq_handler_c(void)
{
    uint32_t pic_status = *PIC_IRQ_STATUS; // Read the interrupt status from the PIC

    uart_puts(uart0, "IRQ Handler\n");

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
        }
    }

    if (pic_status & PIC_TIMERINT1)
    {
        // Handle Timer1 interrupt
        uart_puts(uart0, "Timer1 Interrupt\n");
    }

    if (pic_status & PIC_SOFTINT)
    {
        // Handle software interrupt
        uart_puts(uart0, "Software Interrupt\n");
    }
}