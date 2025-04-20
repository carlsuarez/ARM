#include "timer.h"

void timer0_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    *TIMER0_LOAD = load_value;                                // Load the timer with the initial value
    *TIMER0_CONTROL = mode | ie | prescaler | size | oneshot; // Configure the timer
}

void timer1_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    *TIMER1_LOAD = load_value;                                // Load the timer with the initial value
    *TIMER1_CONTROL = mode | ie | prescaler | size | oneshot; // Configure the timer
}

void timer2_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    *TIMER2_LOAD = load_value;                                // Load the timer with the initial value
    *TIMER2_CONTROL = mode | ie | prescaler | size | oneshot; // Configure the timer
}