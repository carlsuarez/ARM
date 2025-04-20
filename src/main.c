#include <stdint.h>
#include "uart.h"
#include "hw/pic.h"
#include "timer.h"
#include "interrupt.h"

int main(void)
{
    uint8_t i = 0;
    PIC_DISABLE(); // Disable all interrupts
    //*PIC_IRQ_ENABLECLR = PIC_TIMERINT1; // Disable Timer 1 interrupt

    uart0_init(115200);                                                                              // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, 0, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, TIMER_WRAPPING); // Initialize timer1 with 1 second period
    //*PIC_IRQ_ENABLESET = PIC_TIMERINT1;                                            // Enable timer1 interrupt
    TIMER1_START();

    uart_puts(uart0, "Hello world\n"); // Send a message to UART0
    while (1)
    {
        uint32_t ris = timer1->ris; // Read the raw interrupt status from timer1
        if (ris & 0x1)              // Check if the interrupt is set
        {
            timer1->intclr = 0x1;      // Clear the interrupt
            uart_putc(uart0, i + '0'); // Send a message to UART0
            i++;
            if (i == 10)
                i = 0;
        }
    }
    return 0;
}