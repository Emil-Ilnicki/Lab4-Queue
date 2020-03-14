#pragma once

#include <sys/types.h>
#include "utility.h"

typedef struct proc {
    unsigned int arrival_time;
    unsigned int priority;
    unsigned int runtime;
    resource_request resources;
    pid_t pid;
} proc;

typedef struct queue {
    proc process;
    struct queue* next;
} queue;

void push(queue** head, proc process);
proc pop(queue** head);
proc new_process(char* line);