#include "scheduler.h"

int schedule(pcb_t *p)
{
        if (p->status == READY)
                push_q(&ready_queue, p);
                /*
        else if (p->status == SLEEP)
                push_q(&sleep_queue, p);
                */
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        pcb_t *p, *n;

        n = (pcb_t *) pop_q(&ready_queue);
        if (n) {
                p = current_proc;
                c_printf("dispatching proc %d\n", n->pid);
                current_proc = n;
                schedule(p);
        }
}
