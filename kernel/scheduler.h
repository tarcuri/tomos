#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "queue.h"

queue *user_ready_queue[16][3];
queue *ready_queue;
queue *sleep_queue;
queue *kb_queue;

int schedule(pcb_t *p);
void dispatch(void);

void init_scheduler_queues(void);
void set_kb_ready(void);

#endif
