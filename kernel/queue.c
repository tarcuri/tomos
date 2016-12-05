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
                queue *qi = *q;
                while (qi->next)
                        qi = qi->next;

                qi->next = n;
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

int remove_element_q(queue **q, void *data)
{
        queue *head = *q;
        struct q_node *qi = head;
        int found = 0;

        if (qi->data == data) {
                // found it at beginning of queue
                *q = qi->next;
                kfree(qi);
                return 1;
        }

        while (qi) {
                if (qi->data == data) {
                        struct q_node *qp = head;
                        while (qp) {
                                if ((qp->next)->data == data) {
                                        // qp->next is the one we 
                                        // want to remove
                                        qp->next = (qp->next)->next;
                                        kfree(qp);
                                        break;
                                }
                                qp = qp->next;
                        }

                        found = 1;
                        break;
                }

                qi = qi->next;
        }

        return found;
}

void print_q(queue *q)
{

}

int size_q(queue *q)
{
        int size = 0;
        while (q) {
                size++;
                q = q->next;
        }

        return size;
}
