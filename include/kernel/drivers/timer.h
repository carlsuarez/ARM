#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
#include <kernel/hw/timer.h>

#define TIMER0_START() (timer0->control |= TIMER_ENABLE)
#define TIMER0_STOP() (timer0->control &= ~TIMER_ENABLE)
#define TIMER1_START() (timer1->control |= TIMER_ENABLE)
#define TIMER1_STOP() (timer1->control &= ~TIMER_ENABLE)
#define TIMER2_START() (timer2->control |= TIMER_ENABLE)
#define TIMER2_STOP() (timer2->control &= ~TIMER_ENABLE)
void timer0_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot);
void timer1_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot);
void timer2_init(uint32_t load_value, uint8_t mode, uint8_t ie, uint8_t prescaler, uint8_t size, uint8_t oneshot);

#endif