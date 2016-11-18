#include "scheduler.h"
#include "kernel.h"

int proc_kb_ready = 0;

int schedule(pcb_t *p)
{
        if (p->status == READY) {
                push_q(&user_ready_queue[p->uid], p);
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
        static unsigned int urq_idx = 0;
        pcb_t *p, *n;

        if (proc_kb_ready) {
                n = (pcb_t *) pop_q(&kb_queue);
                proc_kb_ready = 0;
        } else {
                int i = 0;
                while (!user_ready_queue[urq_idx]) {
                        urq_idx = ++urq_idx % 16;
                        ++i;
                        if (i == 15)
                                break;
                }
                n = (pcb_t *) pop_q(&user_ready_queue[urq_idx]);
                n = (pcb_t *) pop_q(&ready_queue);
                urq_idx = ++urq_idx % 16;
        }

        if (n) {
                p = current_proc;
                current_proc = n;
                if (p) {
                        schedule(p);
                }
        }
        /* 
         * if we can't get another process, we should see if the current
         * proc is still ready. otherwise we should schedule the idle process.
         */
}

void init_scheduler_queues(void)
{
        int i;
        for (i = 1; i < 16; ++i) {
                user_ready_queue[i] = NULL;
        }
}

void set_kb_ready(void)
{
        proc_kb_ready = 1;
}
