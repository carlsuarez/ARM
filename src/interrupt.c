#include "interrupt.h"
#include "hw/pic.h"
#include "uart.h"
#include <stdint.h>

void identify_and_clear_source(void)
{
    uint32_t pic_status = *PIC_IRQ_STATUS; // Read the interrupt status from the PIC

    uart_puts(uart0, "Identify and clear source\n");

    if (pic_status & PIC_UARTINT0)
    {
        *get_uart_reg(uart0, UART_ICR) = 0x03FF; // Clear the UART0 interrupt
    }

    if (pic_status & PIC_UARTINT1)
    {
        // Clear the UART1 interrupt
        *get_uart_reg(uart1, UART_ICR) = 0x03FF; // Clear the UART1 interrupt
    }
}

void irq_handler_c(void)
{
    uint32_t pic_status = *PIC_IRQ_STATUS; // Read the interrupt status from the PIC

    uart_puts(uart0, "IRQ Handler\n");

    if (pic_status & PIC_UARTINT0)
    {
        // Handle UART0 interrupt
        uint32_t uart_mis = *get_uart_reg(uart0, UART_MIS); // Read the masked interrupt status from UART0

        // Recieve interrupt
        if (uart_mis & UART_MIS_RXMIS)
        {
            // Read the received character
            char c = *get_uart_reg(uart0, UART_DR);
            uart_putc(uart0, c); // Echo the character back
        }
    }

    if (pic_status & PIC_SOFTINT)
    {
        // Handle software interrupt
        uart_puts(uart0, "Software Interrupt\n");
    }
}