#include "mlfq.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "msg.h"

#define MLFQ_LEVELS 3
#define MLFQ_TIMESLICE_MS 500

void mlfq_init(mlfq_t *m) {
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        m->queues[i].head = m->queues[i].tail = NULL;
    }
}

void mlfq_push(mlfq_t *m, pcb_t *p) {
    // New tasks enter at highest priority (0)
    enqueue_pcb(&m->queues[0], p);
}

pcb_t* mlfq_pop(mlfq_t *m, int *from_queue) {
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        pcb_t *p = dequeue_pcb(&m->queues[i]);
        if (p) {
            if (from_queue) *from_queue = i;
            return p;
        }
    }
    return NULL;
}

/*
 * MLFQ scheduler:
 * - uses MLFQ_LEVELS queues
 * - quantum per level = MLFQ_QUANTUM_MS
 * - tasks that consume their quantum are demoted one level (unless already at lowest)
 * - tasks that finish send PROCESS_REQUEST_DONE
 */
void mlfq_scheduler(uint32_t current_time_ms, mlfq_t *mlfq, pcb_t **cpu_task, int *cpu_queue, uint32_t *quantum_used) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        *quantum_used += TICKS_MS;

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free((*cpu_task));
            (*cpu_task) = NULL;
            *quantum_used = 0;
        } else if (*quantum_used >= MLFQ_TIMESLICE_MS) {
            // demote task
            int new_q = (*cpu_queue < MLFQ_LEVELS - 1) ? (*cpu_queue + 1) : *cpu_queue;
            enqueue_pcb(&mlfq->queues[new_q], *cpu_task);
            *cpu_task = NULL;
            *quantum_used = 0;
        }
    }

    if (*cpu_task == NULL) {
        int from = -1;
        pcb_t *next = mlfq_pop(mlfq, &from);
        if (next) {
            *cpu_task = next;
            *cpu_queue = from;
            *quantum_used = 0;
        }
    }
}
