#ifndef HW_TIMER_H
#define HW_TIMER_H
#include <stdint.h>

typedef struct
{
   volatile uint32_t load;    // Load register
   volatile uint32_t value;   // Current value register
   volatile uint32_t control; // Control register
   volatile uint32_t intclr;  // Interrupt clear register
   volatile uint32_t ris;     // Raw interrupt status register
   volatile uint32_t mis;     // Masked interrupt status register
   volatile uint32_t bgload;  // Background load register
} timer_t;

#define TIMER0_BASE 0x13000000 // Base address for Timer 0
#define TIMER1_BASE 0x13000100 // Base address for Timer 1
#define TIMER2_BASE 0x13000200 // Base address for Timer 2

#define timer0 ((timer_t *)TIMER0_BASE) // Timer 0
#define timer1 ((timer_t *)TIMER1_BASE) // Timer 1
#define timer2 ((timer_t *)TIMER2_BASE) // Timer 2

// Timer Control Register Bit Definitions
#define TIMER_ENABLE (1 << 7)           // Enable the timer
#define TIMER_MODE_PERIODIC (1 << 6)    // 1 for periodic, 0 for free running
#define TIMER_MODE_FREE_RUN (0 << 6)    // 1 for periodic, 0 for free running
#define TIMER_IE (1 << 5)               // Enable interrupt
#define TIMER_PRESCALE_NONE_gc (0 << 2) // No prescale
#define TIMER_PRESCALE_16_gc (1 << 2)   // Prescale by 16
#define TIMER_PRESCALE_256_gc (2 << 2)  // Prescale by 256
#define TIMER_SIZE_32 (1 << 1)          // 1 for 32-bit, 0 for 16-bit
#define TIMER_SIZE_16 (0 << 1)          // 1 for 32-bit, 0 for 16-bit
#define TIMER_ONESHOT (1 << 0)          // 1 for one-shot, 0 for wrapping
#define TIMER_WRAPPING (0 << 0)         // 1 for one-shot, 0 for wrapping

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