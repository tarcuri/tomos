#ifndef QUEUE_H
#define QUEUE_H

struct q_node {
        struct q_node *next;
        void *data;
};

typedef struct q_node queue;

void push_q(queue *q, void *data);
void *pop_q(queue **q);

#endif