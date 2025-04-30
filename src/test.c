#include "uart.h"

void test(uint32_t hex)
{
    uart_puts(uart0, "Test function called\n");
    uart_puts(uart0, "Hex value: ");
    uart_puthex(uart0, hex);
    uart_putc(uart0, '\n');
    while (1)
        ;
}