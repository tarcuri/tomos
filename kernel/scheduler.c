#include "scheduler.h"
#include "kernel.h"

int proc_kb_ready = 0;

int schedule(pcb_t *p)
{
        //asm volatile ("cli");
        
        if (p->prio > HIGH || p->prio < LOW) {
                c_printf("invalid prio for %s (%d)\n", p->cmd, p->prio);
                panic("");
        }

        if (p->status == READY) {
                push_q(&user_ready_queue[p->uid][p->prio], p);
        } else if (p->status == KB_WAIT) {
                push_q(&kb_queue, p);
        } else if (p->status == TERMINATE) {
                uint16_t pid = p->pid;

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

                printf("\nterminated process (%d)\n", pid);
        }

        //asm volatile ("sti");
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        static unsigned int urq_idx = 0;
        pcb_t *p, *n;

        p = NULL;
        n = NULL;

        if (proc_kb_ready) {
                n = (pcb_t *) pop_q(&kb_queue);
                proc_kb_ready = 0;
        } else {
                int i = 0;
                int found_proc = 0;
                uint16_t prio = HIGH;
                // check priorities from high to low
                while (!found_proc) {
                        if (size_q(user_ready_queue[urq_idx][HIGH])) {
                                found_proc = 1;
                                n = (pcb_t *) pop_q(&user_ready_queue[urq_idx][HIGH]);
                                break;
                        } else if (size_q(user_ready_queue[urq_idx][MEDIUM])) {
                                found_proc = 1;
                                n = (pcb_t *) pop_q(&user_ready_queue[urq_idx][MEDIUM]);
                                break;
                        } else if (size_q(user_ready_queue[urq_idx][LOW])) {
                                found_proc = 1;
                                //print_q(user_ready_queue[urq_idx][LOW]);
                                n = (pcb_t *) pop_q(&user_ready_queue[urq_idx][LOW]);
                                break;
                        }

                        urq_idx = ++urq_idx % 16;
                        i++;
                        if (i == 16)
                                break;
                }

                if (!found_proc) {
                        //syslog("couldn't find a process\n");
                }

                urq_idx = ++urq_idx % 16;
        }

        if (n) {
                p = current_proc;
                current_proc = n;
                current_proc->wait_t = 0;
                if (p){ // && p != idle_proc) {
                        schedule(p);
                }

        } else if (current_proc->status != READY) {
                //current_proc = idle_proc;
        }

        // update priority queues
        int i, j;
        for (i = 0; i < 16; ++i) {
                // check MED and LOW priority queues,
                // no need to update HIGH priority
                struct q_node *qn = user_ready_queue[i][MEDIUM];
                while (qn) {
                        pcb_t *qp = (pcb_t *) qn->data;
                        qp->wait_t++;
                        if (qp->wait_t >= 3) {
                                remove_element_q(&user_ready_queue[i][MEDIUM], qn->data);
                                push_q(&user_ready_queue[i][HIGH], qp);
                        }
                        qn = qn->next;
                }

                qn = user_ready_queue[i][LOW];
                while (qn) {
                        pcb_t *qp = (pcb_t *) qn->data;
                        qp->wait_t++;
                        if (qp->wait_t >= 6) {
                                remove_element_q(&user_ready_queue[i][LOW], qn->data);
                                push_q(&user_ready_queue[i][MEDIUM], qp);
                        }
                        qn = qn->next;
                }
        }
        //c_printf("exit dispatch\n");

}

void init_scheduler_queues(void)
{
        int i;
        for (i = 1; i < 16; ++i) {
                user_ready_queue[i][LOW] = NULL;
                user_ready_queue[i][MEDIUM] = NULL;
                user_ready_queue[i][HIGH] = NULL;
        }
}

void set_kb_ready(void)
{
        proc_kb_ready = 1;
}
