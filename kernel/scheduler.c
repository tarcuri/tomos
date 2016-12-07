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
                //c_printf("scheduled %s (prio: %d)\n", p->cmd, p->prio);
                //print_q(user_ready_queue[p->uid][p->prio]);
        } else if (p->status == KB_WAIT) {
                //c_printf("pushed %s to kb_queue\n", p->cmd);
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
                //c_printf("kb ready, popped %s (%s)\n", n->cmd,
                //        proc_status_string(p->status));
                proc_kb_ready = 0;
        } else {
                int i = 0;
                int found_proc = 0;
                uint16_t prio = HIGH;
                // check priorities from high to low
                //c_printf("urq_idx: %d\n", urq_idx);
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

                /*
                c_printf("i: %d\n", i);
                if (n == NULL)
                        c_printf("couldn't find a proc\n");
                else
                c_printf("got process %s\n", n->cmd);
                c_printf("current_proc: %s\n", current_proc->cmd);

                print_q(user_ready_queue[urq_idx][HIGH]);
                print_q(user_ready_queue[urq_idx][MEDIUM]);
                print_q(user_ready_queue[urq_idx][LOW]);

                if ((strcmp(current_proc->cmd, "tomsh") == 0))
                        panic("");
                */
                if (!found_proc) {
                        //syslog("couldn't find a process\n");
                }

                urq_idx = ++urq_idx % 16;
        }

        if (n) {
                p = current_proc;
                current_proc = n;
                //char msg[512];
                //snprintf(msg, 512, "dispatching %s [%d]\n",
                //                current_proc->cmd, current_proc->pid);
                //syslog(msg);
                current_proc->wait_t = 0;
                /*
                c_printf(msg);
                if (strcmp(current_proc->cmd, "idle") == 0)
                        panic("");
                if ((strcmp(p->cmd, "tomsh") == 0) &&
                                strcmp(current_proc->cmd, "idle") == 0)
                        panic("");
                */
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
