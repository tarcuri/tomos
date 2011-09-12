#ifndef CLOCK_H
#define CLOCK_H

static unsigned long long clock_ticks;

void clock_init(void);
static void timer_isr(int,int);

#endif
