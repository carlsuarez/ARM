#include "test.h"

void test(uint32_t hex)
{
    uart_puts(uart0, "Test function called\n");
    uart_puts(uart0, "Hex value: ");
    uart_puthex(uart0, hex);
    uart_putc(uart0, '\n');
}

void dummy(void)
{
    uart_puts(uart0, "Dummy function called\n");
    dummy2();
    for (volatile uint32_t i = 0; i < 1e9; i++)
        ;
}

void dummy2(void)
{
    uart_puts(uart0, "Dummy2 function called\n");
    for (volatile uint32_t i = 0; i < 1e9; i++)
        ;
}