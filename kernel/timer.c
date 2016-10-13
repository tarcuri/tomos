#include "timer.h"
#include "interrupt.h"
#include "x86.h"

uint32_t get_time(void)
{
        return system_time;
}

void timer_init(void)
{
        system_time = 0;

        // clock will tick at CLOCK_FREQUENCY_HZ Hz
        __outb(TIMER_0_PORT, (TIMER_FREQUENCY / CLOCK_FREQUENCY_HZ) & 0xff);
        __outb(TIMER_1_PORT, (TIMER_FREQUENCY / CLOCK_FREQUENCY_HZ) >> 8);

        _install_isr(INT_VEC_TIMER, timer_isr);

        c_printf("[timer]   PIT timer initializedn\n");
}

void timer_isr(int vector, int code)
{
        ++system_time;

        __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}
