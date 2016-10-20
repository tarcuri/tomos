#include "scheduler.h"

int schedule(pcb_t *p)
{
        push_q(ready_queue, p);
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        pcb_t *p = current_proc;

        current_proc = (pcb_t *) pop_q(&ready_queue);
        schedule(p);
}