#include "sjf.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "msg.h"


void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
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
        }
    }

    if (*cpu_task == NULL) {
        queue_elem_t *prev = NULL;
        queue_elem_t *cur = rq->head;
        queue_elem_t *prev_shortest = NULL;
        queue_elem_t *shortest = NULL;

        while (cur) {
            if (!shortest || cur->pcb->time_ms < shortest->pcb->time_ms) {
                shortest = cur;
                prev_shortest = prev;
            }
            prev = cur;
            cur = cur->next;
        }

        if (shortest) {
            if (prev_shortest) {
                prev_shortest->next = shortest->next;
            } else {
                rq->head = shortest->next;
            }
            if (rq->tail == shortest) {
                rq->tail = prev_shortest;
            }
            pcb_t *chosen = shortest->pcb;
            shortest->pcb = NULL;
            shortest->next = NULL;
            free(shortest);
            *cpu_task = chosen;
        }
    }
}
