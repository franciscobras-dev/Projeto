#ifndef MLFQ_H
#define MLFQ_H
#include "queue.h"
#include <stdint.h>

#define NUM_QUEUES 3
extern queue_t mlfq_queues[NUM_QUEUES];

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);
void mlfq_enqueue(pcb_t* task); // função auxiliar para colocar na fila 0

#endif