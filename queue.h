#pragma once

#include "utility.h"
#include <sys/types.h>
#include <stdbool.h>

typedef struct proc {
    unsigned int arrival_time;
    unsigned int priority;
    unsigned int runtime;
    resource_request resources;
    pid_t pid;
    int address;
} proc;

typedef struct queue {
    proc* process;
    struct queue* next;
} queue;

void push(queue** head, proc* process);
proc* pop(queue** head);
proc* new_process(char* line);

bool equals(proc p1, proc p2);
void print_process(proc p);