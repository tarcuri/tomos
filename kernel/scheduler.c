#include "scheduler.h"

int schedule(pcb_t *p)
{
        if (p->status == READY) {
                push_q(&ready_queue, p);
        } else if (p->status == TERMINATE) {
                uint16_t pid = p->pid;

                asm volatile ("cli");

                pcb_t *pcb;
                for (pcb = pcb_list; pcb; pcb = pcb->next) {
                        if (pcb == p) {
                                if (pcb->prev)
                                        (pcb->prev)->next = pcb->next;
                                if (pcb->next)
                                        (pcb->next)->prev = pcb->prev;
                                break;
                        }
                }

                kfree(p->context);
                kfree(p->stack);
                kfree(p);

                asm volatile ("sti");

                printf("\nterminated process (%d)\n", pid);
        }
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        pcb_t *p, *n;

        n = (pcb_t *) pop_q(&ready_queue);
        if (n) {
                p = current_proc;
                //c_printf("dispatching proc %d\n", n->pid);
                current_proc = n;
                schedule(p);
        }
}
