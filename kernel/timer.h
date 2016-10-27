#ifndef TIMER_H
#define TIMER_H

#include "kernel/process.h"
#include <stdint.h>

#define CLOCK_FREQUENCY_HZ      100

struct timer {
        struct timer    *next;
        struct timer    *prev;
        uint32_t        delay;
        uint32_t        start;
        uint32_t        current;
        uint8_t         expired;
        pcb_t           *proc;
};

struct timer *proc_sleep_timers;
uint32_t system_time;

uint32_t get_time(void);

struct timer *sleep_timer(uint32_t delay);
void del_sleep_timer(struct timer *t);

void timer_isr(int vector, int code);

void timer_init(void);

#endif
