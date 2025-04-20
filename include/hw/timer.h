#ifndef HW_TIMER_H
#define HW_TIMER_H
#include <stdint.h>

// Timer Control Register Bit Definitions
#define TIMER0_LOAD ((volatile uint32_t *)0x13000000)
#define TIMER0_VALUE ((volatile uint32_t *)0x13000004)
#define TIMER0_CONTROL ((volatile uint32_t *)0x13000008)
#define TIMER0_INTCLR ((volatile uint32_t *)0x1300000C)
#define TIMER0_RIS ((volatile uint32_t *)0x13000010)
#define TIMER0_MIS ((volatile uint32_t *)0x13000014)
#define TIMER0_BGLOAD ((volatile uint32_t *)0x13000018)

#define TIMER1_LOAD ((volatile uint32_t *)0x13000100)
#define TIMER1_VALUE ((volatile uint32_t *)0x13000104)
#define TIMER1_CONTROL ((volatile uint32_t *)0x13000108)
#define TIMER1_INTCLR ((volatile uint32_t *)0x1300010C)
#define TIMER1_RIS ((volatile uint32_t *)0x13000110)
#define TIMER1_MIS ((volatile uint32_t *)0x13000114)
#define TIMER1_BGLOAD ((volatile uint32_t *)0x13000118)

#define TIMER2_LOAD ((volatile uint32_t *)0x13000200)
#define TIMER2_VALUE ((volatile uint32_t *)0x13000204)
#define TIMER2_CONTROL ((volatile uint32_t *)0x13000208)
#define TIMER2_INTCLR ((volatile uint32_t *)0x1300020C)
#define TIMER2_RIS ((volatile uint32_t *)0x13000210)
#define TIMER2_MIS ((volatile uint32_t *)0x13000214)
#define TIMER2_BGLOAD ((volatile uint32_t *)0x13000218)

// Timer Control Register Bit Definitions
#define TIMER_ENABLE (1 << 7)           // Enable the timer
#define TIMER_MODE (1 << 6)             // 1 for periodic, 0 for free running
#define TIMER_IE (1 << 5)               // Enable interrupt
#define TIMER_PRESCALE_NONE_gc (0 << 2) // No prescale
#define TIMER_PRESCALE_16_gc (1 << 2)   // Prescale by 16
#define TIMER_PRESCALE_256_gc (2 << 2)  // Prescale by 256
#define TIMER_SIZE (1 << 1)             // 1 for 32-bit, 0 for 16-bit
#define TIMER_ONESHOT (1 << 0)          // 1 for one-shot, 0 for wrapping

/*
    Timer 0 is clocked by the system bus clock.
    Timer 1 and Timer 2 are clocked at a fixed frequency of 1MHz.

    The timers provide three operating modes:

    1. Free running:
       - The timer counts down to zero and then wraps around.
       - It continues to count down from its maximum value.

    2. Periodic:
       - The counter counts down to zero.
       - It then reloads the period value held in the load register.
       - The counter continues to decrement.

    3. One shot:
       - The counter counts down to zero.
       - It does not reload a value.
*/

#endif