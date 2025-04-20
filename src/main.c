#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"

int main(void)
{
    uint8_t i = 0;
    PIC_DISABLE(); // Disable all interrupts

    uart0_init(115200);                                                     // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e9, TIMER_MODE, 0, TIMER_PRESCALE_NONE_gc, TIMER_SIZE, 0); // Initialize timer1 with 1 second period
    TIMER1_START();

    uart_puts(uart0, "Hello world\n"); // Send a message to UART0
    while (1)
    {

        uint32_t ris = *TIMER1_RIS; // Read the raw interrupt status from timer1
        if (ris & 0x1)              // Check if the timer1 interrupt is set
        {
            *TIMER1_INTCLR = 0x1;      // Clear the timer1 interrupt
            uart_putc(uart0, i + '0'); // Send the current value of i to UART0
            uart_putc(uart0, '\n');    // Send a newline character to UART0

            if (i == 9) // If i is 9, reset it to 0
                i = 0;
            else
                i++; // Increment i
        }
    }

    return 0;
}