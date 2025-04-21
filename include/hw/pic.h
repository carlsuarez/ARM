#ifndef PIC_H
#define PIC_H

// PIC (Primary Interrupt Controller) register definitions
#define PIC_BASE 0x14000000

typedef struct
{
    volatile uint32_t IRQ_STATUS;    // 0x00
    volatile uint32_t IRQ_RAWSTAT;   // 0x04
    volatile uint32_t IRQ_ENABLESET; // 0x08
    volatile uint32_t IRQ_ENABLECLR; // 0x0C
    volatile uint32_t INT_SOFTSET;   // 0x10
    volatile uint32_t INT_SOFTCLR;   // 0x14
    volatile uint32_t _reserved[2];  // 0x18 - 0x1C
    volatile uint32_t FIQ_STATUS;    // 0x20
    volatile uint32_t FIQ_RAWSTAT;   // 0x24
    volatile uint32_t FIQ_ENABLESET; // 0x28
    volatile uint32_t FIQ_ENABLECLR; // 0x2C
} pic_t;

#define pic ((pic_t *)PIC_BASE)

#define PIC_IRQ_DISABLE() (pic->IRQ_ENABLECLR =                                             \
                               PIC_TS_PENINT | PIC_ETH_INT | PIC_CPPLDINT | PIC_AACIINT |   \
                               PIC_MMCIINT1 | PIC_MMCIINT0 | PIC_CLCDCINT | PIC_LM_LLINT1 | \
                               PIC_LM_LLINT0 | PIC_RTCINT | PIC_TIMERINT2 | PIC_TIMERINT1 | \
                               PIC_TIMERINT0 | PIC_MOUSEINT | PIC_KBDINT | PIC_UARTINT1 |   \
                               PIC_UARTINT0 | PIC_SOFTINT)

#define PIC_FIQ_DISABLE() (pic->FIQ_ENABLECLR =                                             \
                               PIC_TS_PENINT | PIC_ETH_INT | PIC_CPPLDINT | PIC_AACIINT |   \
                               PIC_MMCIINT1 | PIC_MMCIINT0 | PIC_CLCDCINT | PIC_LM_LLINT1 | \
                               PIC_LM_LLINT0 | PIC_RTCINT | PIC_TIMERINT2 | PIC_TIMERINT1 | \
                               PIC_TIMERINT0 | PIC_MOUSEINT | PIC_KBDINT | PIC_UARTINT1 |   \
                               PIC_UARTINT0 | PIC_SOFTINT)

// PIC interrupt status bits
#define PIC_TS_PENINT (1 << 28)
#define PIC_ETH_INT (1 << 27)
#define PIC_CPPLDINT (1 << 26)
#define PIC_AACIINT (1 << 25)
#define PIC_MMCIINT1 (1 << 24)
#define PIC_MMCIINT0 (1 << 23)
#define PIC_CLCDCINT (1 << 22)
#define PIC_LM_LLINT1 (1 << 10)
#define PIC_LM_LLINT0 (1 << 9)
#define PIC_RTCINT (1 << 8)
#define PIC_TIMERINT2 (1 << 7)
#define PIC_TIMERINT1 (1 << 6)
#define PIC_TIMERINT0 (1 << 5)
#define PIC_MOUSEINT (1 << 4)
#define PIC_KBDINT (1 << 3)
#define PIC_UARTINT1 (1 << 2)
#define PIC_UARTINT0 (1 << 1)
#define PIC_SOFTINT (1 << 0)

#endif
