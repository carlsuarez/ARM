#ifndef PL011_H
#define PL011_H
#include <stdint.h>
#include <stddef.h>

struct pl011_t
{
    uint64_t base;      // Base address of the PL011 UART
    uint32_t baud_rate; // Baud rate for the UART
    uint32_t clock;     // Clock frequency for the UART
    uint32_t data_bits; // Number of data bits (5-8)
    uint32_t stop_bits; // Number of stop bits (1 or 2)
    uint32_t parity;    // Parity (0: none, 1: odd, 2: even)
};

#define PL011_BASE 0x16000000 // Base address for PL011 UART
#define PL011_CLOCK 24000000  // Clock frequency for PL011 UART

// Register offsets
#define UART_DR 0x00    // Data Register
#define UART_RSR 0x04   // Receive Status Register
#define UART_ECR 0x04   // Error Clear Register
#define UART_FR 0x18    // Flag Register
#define UART_IBRD 0x24  // Integer Baud Rate Register
#define UART_FBRD 0x28  // Fractional Baud Rate Register
#define UART_LCRH 0x2C  // Line Control Register
#define UART_CR 0x30    // Control Register
#define UART_IFLS 0x34  // Interrupt FIFO Level Select Register
#define UART_IMSC 0x38  // Interrupt Mask Set/Clear Register
#define UART_RIS 0x3C   // Raw Interrupt Status Register
#define UART_MIS 0x40   // Masked Interrupt Status Register
#define UART_ICR 0x44   // Interrupt Clear Register
#define UART_DMACR 0x48 // DMA Control Register

// Flag Register bits
#define UART_FR_TXFF (1 << 5) // Transmit FIFO full
#define UART_FR_RXFE (1 << 4) // Receive FIFO empty
#define UART_FR_BUSY (1 << 3) // UART busy
#define UART_FR_CTS (1 << 0)  // Clear to Send

// Line Control Register bits
#define UART_LCRH_WLEN_5_gc (0 << 5) // 5 data bits
#define UART_LCRH_WLEN_6_gc (1 << 5) // 6 data bits
#define UART_LCRH_WLEN_7_gc (2 << 5) // 7 data bits
#define UART_LCRH_WLEN_8_gc (3 << 5) // 8 data bits
#define UART_LCRH_FEN (1 << 4)       // Enable FIFOs
#define UART_LCRH_STP2 (1 << 3)      // Two stop bits
#define UART_LCRH_EPS (1 << 2)       // Even parity select
#define UART_LCRH_PEN (1 << 1)       // Parity enable

// Control Register bits
#define UART_CR_CTSEN (1 << 15) // CTS hardware flow control enable
#define UART_CR_RTSEN (1 << 14) // RTS hardware flow control enable
#define UART_CR_OUT2 (1 << 13)  // Out2
#define UART_CR_OUT1 (1 << 12)  // Out1
#define UART_CR_RTS (1 << 11)   // RTS
#define UART_CR_DTR (1 << 10)   // DTR
#define UART_CR_RXE (1 << 9)    // Receive enable
#define UART_CR_TXE (1 << 8)    // Transmit enable
#define UART_CR_LBE (1 << 7)    // Loopback enable
#define UART_CR_UARTEN (1 << 0) // UART enable

#endif