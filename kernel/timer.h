#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define CLOCK_FREQUENCY_HZ      100

uint32_t system_time;

uint32_t get_time(void);

void timer_isr(int vector, int code);

void timer_init(void);

#endif
