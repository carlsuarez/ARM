#include "drivers/timer.h"

void timer0_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    timer0->load = load_value;                                // Load the timer with the initial value
    timer0->control = mode | ie | prescaler | size | oneshot; // Configure the timer
}

void timer1_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    timer1->load = load_value;                                // Load the timer with the initial value
    timer1->control = mode | ie | prescaler | size | oneshot; // Configure the timer
}

void timer2_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot)
{
    timer2->load = load_value;                                // Load the timer with the initial value
    timer2->control = mode | ie | prescaler | size | oneshot; // Configure the timer
}