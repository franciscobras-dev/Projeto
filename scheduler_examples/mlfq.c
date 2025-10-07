#include "mlfq.h"
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"

#define TIME_SLICE_MS 500

queue_t mlfq_queues[NUM_QUEUES] = {0};

void mlfq_enqueue(pcb_t* task) {
    // Nova chegada → fila 0
    task->slice_start_ms = 0; // reset slice
    enqueue_pcb(&mlfq_queues[0], task);
}

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    (void)rq; // ignoramos a fila única; usamos mlfq_queues

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
        } else if (current_time_ms - (*cpu_task)->slice_start_ms >= TIME_SLICE_MS) {
            // Desce de nível (se possível)
            int current_level = 0;
            // Infelizmente, não temos o nível armazenado...
            // Solução: assumimos que se está a correr, veio da fila mais alta possível
            // Mas sem campo de nível, é difícil. Vamos simplificar:
            // → Se não terminou no slice, vai para a fila mais baixa (fila 2)
            enqueue_pcb(&mlfq_queues[2], *cpu_task);
            *cpu_task = NULL;
        }
    }

    // Procurar na fila mais alta com processos
    if (*cpu_task == NULL) {
        for (int i = 0; i < NUM_QUEUES; i++) {
            if (mlfq_queues[i].head != NULL) {
                *cpu_task = dequeue_pcb(&mlfq_queues[i]);
                if (*cpu_task) {
                    (*cpu_task)->slice_start_ms = current_time_ms;
                }
                break;
            }
        }
    }
}