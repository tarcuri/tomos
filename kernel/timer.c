#include "timer.h"
#include "scheduler.h"
#include "interrupt.h"
#include "heap.h"
#include "x86.h"

uint32_t get_time(void)
{
        return system_time;
}

void add_sleep_timer(uint32_t delay)
{
        asm volatile ("cli");
        
        struct timer *t = (struct timer *) kmalloc(sizeof(struct timer));
        
        t->delay = delay;
        t->start = system_time;
        t->next = NULL;
        t->proc = current_proc;

        if (!proc_sleep_timers) {
                proc_sleep_timers = t;
                t->prev = NULL;
        } else {
                struct timer *h = proc_sleep_timers;
                while (h->next)
                        h = h->next;

                t->prev = h;
                h->next = t;
        }

        t->proc->status = SLEEP;
        schedule(current_proc);

        asm volatile ("sti");
}

void del_sleep_timer(struct timer *t)
{
        asm volatile ("cli");

        struct timer *h;

        for (h = proc_sleep_timers; h; h = h->next) {
                if (h != t)
                        continue;

                if (h->prev)
                        (h->prev)->next = h->next;
                if (h->next)
                        (h->next)->prev = h->prev;
                if (proc_sleep_timers == h)
                        proc_sleep_timers = h->next;

                kfree(t);
                break;
        }

        asm volatile ("sti");
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

        struct timer *t;
        for (t = proc_sleep_timers; t; t = t->next) {
                t->current = system_time;
                if (t->current - t->start >= t->delay) {
                        t->proc->status = READY;
                        schedule(t->proc);
                        del_sleep_timer(t);
                }
        }

        if (system_time % 100 == 0) {
                dispatch();
        }

        __outb(PIC_MASTER_CMD_PORT, PIC_EOI);
}
