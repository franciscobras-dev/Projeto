// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "scheduler_examples/fifo.h"
#include "scheduler_examples/sjf.h"
#include "scheduler_examples/rr.h"
#include "scheduler_examples/mlfq.h"
#include "scheduler_examples/queue.h"
#include "scheduler_examples/msg.h"

#ifndef TICKS_MS
#define TICKS_MS 10
#endif

void simulate_tick() {
    usleep(TICKS_MS * 1000); // 10 ms = 10000 µs
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <algorithm> (fifo|sjf|rr|mlfq)\n", argv[0]);
        return 1;
    }

    char* algo = argv[1];
    queue_t ready_queue = {0};
    pcb_t* cpu_task = NULL;
    uint32_t current_time_ms = 0;

    void (*scheduler)(uint32_t, queue_t*, pcb_t**) = NULL;

    if (strcmp(algo, "fifo") == 0) {
        scheduler = fifo_scheduler;
    } else if (strcmp(algo, "sjf") == 0) {
        scheduler = sjf_scheduler;
    } else if (strcmp(algo, "rr") == 0) {
        scheduler = rr_scheduler;
    } else if (strcmp(algo, "mlfq") == 0) {
        scheduler = mlfq_scheduler;
    } else {
        fprintf(stderr, "Unknown algorithm: %s\n", algo);
        return 1;
    }

    printf("Simulator started with algorithm: %s\n", algo);
    printf("Enter processes as: <pid> <time_ms> (or 'quit' to stop input)\n");

    while (1) {
        if (feof(stdin)) break;

        static int input_done = 0;
        if (!input_done) {
            char line[100];
            if (fgets(line, sizeof(line), stdin) != NULL) {
                if (strncmp(line, "quit", 4) == 0) {
                    input_done = 1;
                } else {
                    pid_t pid;
                    uint32_t time_ms;
                    if (sscanf(line, "%d %u", &pid, &time_ms) == 2) {
                        pcb_t* new_task = new_pcb(pid, -1, time_ms); // sockfd = -1 (não usado aqui)
                        if (new_task) {
                            if (strcmp(algo, "mlfq") == 0) {
                                mlfq_enqueue(new_task);
                            } else {
                                enqueue_pcb(&ready_queue, new_task);
                            }
                            printf("[t=%u] Process %d arrived (CPU burst: %u ms)\n",
                                   current_time_ms, pid, time_ms);
                        }
                    }
                }
            }
        }

        scheduler(current_time_ms, &ready_queue, &cpu_task);

        if (cpu_task) {
            printf("[t=%u] CPU running PID %d (elapsed: %u / %u)\n",
                   current_time_ms,
                   cpu_task->pid,
                   cpu_task->ellapsed_time_ms,
                   cpu_task->time_ms);
        }

        current_time_ms += TICKS_MS;

        if (input_done && cpu_task == NULL && ready_queue.head == NULL) {
            if (strcmp(algo, "mlfq") == 0) {
                int work = 0;
                for (int i = 0; i < 3; i++) {
                    if (mlfq_queues[i].head) { work = 1; break; }
                }
                if (!work) break;
            } else {
                break;
            }
        }

    }

    printf("Simulation ended at t = %u ms\n", current_time_ms);
    return 0;
}