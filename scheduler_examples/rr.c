#include "rr.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "msg.h"

#define RR_TIMESLICE_MS 500

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task, uint32_t *quantum_used) {
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
        } else if (*quantum_used >= RR_TIMESLICE_MS) {
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;
            *quantum_used = 0;
        }
    }

    if (*cpu_task == NULL) {
        pcb_t *next = dequeue_pcb(rq);
        if (next) {
            *cpu_task = next;
            *quantum_used = 0;
        }
    }
}
