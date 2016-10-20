#include "queue.h"
#include "heap.h"

void push_q(queue *q, void *data)
{
        struct q_node *n = (struct q_node *) kmalloc(sizeof(struct q_node));

        queue *i = q;
        while (i->next)
                i = i->next;

        i->next = n;
        n->next = NULL;
        n->data = data;
}

void *pop_q(queue **q)
{
        void *data = NULL;

        if (q && *q) {
                data = (*q)->data;
                q = (*q)->next;
        }

        return data;
}
