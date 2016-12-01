#include "scheduler.h"
#include "kernel.h"

int proc_kb_ready = 0;

int schedule(pcb_t *p)
{
        asm volatile ("cli");

        if (p->status == READY) {
                push_q(&user_ready_queue[p->uid][p->prio], p);
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

        asm volatile ("sti");
}

// works IFF called from interrupt context (timer_isr)
void dispatch(void)
{
        static unsigned int urq_idx = 0;
        pcb_t *p, *n;

        if (proc_kb_ready) {
                n = (pcb_t *) pop_q(&kb_queue);
                //c_printf("kb ready, popped %s (%s)\n", n->cmd,
                //        proc_status_string(p->status));
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
                urq_idx = ++urq_idx % 16;
        }

        if (n) {
                p = current_proc;
                current_proc = n;
                char msg[512];
                snprintf(msg, 512, "dispatching %s [%d]\n",
                                current_proc->cmd, current_proc->pid);
                syslog(msg);
                if (p){ // && p != idle_proc) {
                        schedule(p);
                }
        } else if (current_proc->status != READY) {
                //current_proc = idle_proc;
        }

        // update priority queues
        int i, j;
        for (i = 0; i < 16; ++i) {
                queue *q = user_process_queue[i];
                for (j = 0; j < 3; ++j) {
                        struct q_node *qn = user_process_queue[i][j];
                        while (qn) {
                                pcb_t *qp = (pcb_t *) qn->data;
                                qp->wait_t++;
                                qn = qn->next;
                        }
                }
        }

}

void init_scheduler_queues(void)
{
        int i;
        for (i = 1; i < 16; ++i) {
                user_ready_queue[i][LOW] = NULL;
                user_ready_queue[i][MED] = NULL;
                user_ready_queue[i][HIGH] = NULL;
        }
}

void set_kb_ready(void)
{
        proc_kb_ready = 1;
}
