#include "scheduler.h"
#include "kernel.h"

int proc_kb_ready = 0;

int schedule(pcb_t *p)

{
        if (p->status == READY) {
                push_q(&ready_queue, p);
        } else if (p->status == KB_WAIT) {
                push_q(&kb_queue, p);
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

        struct q_node *q = (struct q_node *) ready_queue;
        int i = 0;
        char log_msg[1024];

        while (q) {
                pcb_t *qp = (pcb_t *) q->data;
                snprintf(log_msg, 1024, "ready_queue %d: %s (%d)\n",
                                i++, qp->cmd, qp->pid);
                syslog(log_msg);
                q = q->next;
        }

        if (proc_kb_ready) {
                n = (pcb_t *) pop_q(&kb_queue);
                proc_kb_ready = 0;
        } else {
                n = (pcb_t *) pop_q(&ready_queue);
        }

        if (n) {
                p = current_proc;
                current_proc = n;
                if (p) {
                        schedule(p);
                }
        }
}

void set_kb_ready(void)
{
        proc_kb_ready = 1;
}
