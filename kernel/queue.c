#include "queue.h"
#include "heap.h"

void push_q(queue **q, void *data)
{
        if (!q)
                return;

        struct q_node *n = (struct q_node *) kmalloc(sizeof(struct q_node));

        if (*q == NULL) {
                *q = n;
                (*q)->next = NULL;
                (*q)->data = data;
        } else { 
                while ((*q)->next)
                        *q = (*q)->next;

                (*q)->next = n;
                n->next = NULL;
                n->data = data;
        }
}

void *pop_q(queue **q)
{
        struct q_node *t;
        void *data = NULL;

        if (q && *q) {
                t = *q;
                data = t->data;
                *q = t->next;
                kfree(t);
        }

        return data;
}

int remove_element_q(queue **q, struct q_node *qn)
{
        queue *head = *q;
        struct q_node *qi = head;
        int found = 0;

        if (qi == qn) {
                // found it at beginning of queue
                q = qi->next;
                return 1;
        }

        while (qi)
                if (qi == qn) {
                        struct q_node *qp = head;
                        while (qp) {
                                if (qp->next == qi) {
                                        // qp->next is the one we 
                                        // want to remove
                                        qp->next = (qp->next)->next;
                                }
                        }
                }

                qi = qi->next;
        }
}
