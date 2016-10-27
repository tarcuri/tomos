#include "scheduler.h"

int schedule(pcb_t *p)
{
        push_q(&ready_queue, p);
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        pcb_t *p, *n;

        n = (pcb_t *) pop_q(&ready_queue);
        if (n) {
                p = current_proc;
                c_printf("dispatching pcb %p\n", n);
                c_printf("n->context->eip = 0x%x\n", n->context->eip);
                current_proc = n;
                schedule(p);
        }
}
