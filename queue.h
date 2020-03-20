#pragma once

// Include files required for this structure to work correctly
#include "utility.h"
#include <sys/types.h>
#include <stdbool.h>

/*
 * Define a procedure structure that will absorb resources
 * - Based on manual
 */
typedef struct proc {
    unsigned int arrival_time;
    unsigned int priority;
    unsigned int runtime;
    resource_request resources;
    pid_t pid;
    int address;
} proc;

/*
 * Define a queue structure
 */
typedef struct queue {
    proc* process;
    struct queue* next;
} queue;

// Include standard function definitions for these 2 structures above main()
void push(queue** head, proc* process);
proc* pop(queue** head);
proc* new_process(char* line);

bool equals(proc p1, proc p2);
void print_process(proc p);