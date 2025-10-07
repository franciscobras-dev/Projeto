#include "sjf.h"
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Se há um processo a correr, atualiza o tempo
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Terminou
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free(*cpu_task);
            *cpu_task = NULL;
        }
    }

    // Se CPU está livre, escolhe o processo com menor time_ms
    if (*cpu_task == NULL && rq->head != NULL) {
        // Encontrar o processo com menor time_ms
        queue_elem_t *best_elem = rq->head;
        queue_elem_t *prev_best = NULL;
        queue_elem_t *curr = rq->head->next;
        queue_elem_t *prev = rq->head;

        while (curr != NULL) {
            if (curr->pcb->time_ms < best_elem->pcb->time_ms) {
                best_elem = curr;
                prev_best = prev;
            }
            prev = curr;
            curr = curr->next;
        }

        // Remover o melhor da fila
        if (prev_best == NULL) {
            // É o primeiro
            rq->head = best_elem->next;
            if (rq->head == NULL) rq->tail = NULL;
        } else {
            prev_best->next = best_elem->next;
            if (best_elem == rq->tail) rq->tail = prev_best;
        }

        *cpu_task = best_elem->pcb;
        free(best_elem);
    }
}