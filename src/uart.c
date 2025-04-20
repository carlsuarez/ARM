#include "uart.h"

static void calculate_divisiors(uint32_t baud_rate, uint32_t *integer, uint32_t *fractional)
{
    // Want: div = 4 * F_UARTCLK / baud_rate;
    // Use 64-bit math to avoid overflow and do fixed-point math.

    uint64_t temp = ((uint64_t)4 * PL011_CLOCK << 6); // Multiply by 64 (shift left 6) to preserve fractional part
    uint32_t div = (uint32_t)(temp / baud_rate);      // Result is fixed-point: 6 fractional bits

    *fractional = div & 0x3f;       // Lower 6 bits = fractional part
    *integer = (div >> 6) & 0xffff; // Upper bits = integer part
}

static void wait_tx_complete(pl011_t *dev)
{
    while (uart0->fr & UART_FR_BUSY)
        ;
}

static int pl011_reset(pl011_t *dev, uint32_t baud_rate)
{
    uint32_t cr = dev->cr;
    uint32_t lcrh = dev->lcrh;
    uint32_t ibrd, fbrd;

    // Disable UART before anything else
    dev->cr = (cr & ~UART_CR_UARTEN);

    // Wait for any ongoing transmissions to complete
    wait_tx_complete(dev);

    // Flush FIFOs
    dev->lcrh = (lcrh & ~UART_LCRH_FEN);

    // Set frequency divisors (UARTIBRD and UARTFBRD) to configure the speed
    calculate_divisiors(baud_rate, &ibrd, &fbrd);
    dev->ibrd = ibrd;
    dev->fbrd = fbrd;

    // Set line control register (UARTLCRH) to configure data format
    // 8 data bits, 1 stop bit, no parity
    dev->lcrh = UART_LCRH_WLEN_8_gc | UART_LCRH_STP2;

    // Enable recieve interrupt
    dev->icr = UART_IMSC_RXIM;
    dev->imsc &= ~UART_IMSC_RXIM;

    // Trigger recieve interrupt at 7/8 FIFO level
    dev->ifls = UART_IFLS_RXIFLSEL_7_8th_gc;

    // Disable DMA by setting all bits to 0
    dev->dmacr = 0x0;

    // Enable transmission and recieve
    dev->cr = UART_CR_TXE | UART_CR_RXE;

    // Enable UART
    dev->cr = UART_CR_UARTEN;

    return 0;
}

int uart0_init(uint32_t baud_rate)
{
    return pl011_reset(uart0, baud_rate);
}

int uart1_init(uint32_t baud_rate)
{
    return pl011_reset(uart1, baud_rate);
}

void uart_putc(pl011_t *dev, char c)
{
    // Wait until TX FIFO is not full
    wait_tx_complete(dev);

    // Write to data register
    dev->dr = c;
}

void uart_puts(pl011_t *dev, const char *s)
{
    while (*s)
        uart_putc(dev, *s++);
}