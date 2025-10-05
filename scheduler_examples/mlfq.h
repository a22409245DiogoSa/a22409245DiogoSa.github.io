#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"

typedef struct {
    queue_t queues[3];
} mlfq_t;

void mlfq_init(mlfq_t *m);
void mlfq_push(mlfq_t *m, pcb_t *p); // push new task to highest queue
pcb_t* mlfq_pop(mlfq_t *m, int *from_queue);

void mlfq_scheduler(uint32_t current_time_ms, mlfq_t *mlfq, pcb_t **cpu_task, int *cpu_queue, uint32_t *quantum_used);

#endif //MLFQ_H
