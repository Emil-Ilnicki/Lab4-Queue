#include "queue.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void push(queue** head, proc* process) {

    queue* next_node = malloc(sizeof(queue));
    *next_node = (queue) {
            .process=process,
            .next = NULL,
    };

    if (*head == NULL) {
        *head = next_node;
    } else {

        queue* current = *head;
        while (current->next != NULL){
            current = current->next;
        }

        current->next = next_node;
        
    }
}

proc* pop(queue** head) {

    queue* q = (*head);
    if (q == NULL) {
        fprintf(stderr, "Attempted to pop null queue\n");
        exit(1);
    }
    
    proc* p = q->process;
    *head = q->next;
    free(q);

    return p;

}


proc* new_process(char* line) {
    char* token = strtok(line, ", ");
    char* tokens[8];
    int i = 0;
    while( token != NULL ) {
        tokens[i++] = token;
        token = strtok(NULL, ", ");
    }

    proc* p = malloc(sizeof(proc));
    *p = (proc) {
        .arrival_time = atoi(tokens[0]),
        .priority = atoi(tokens[1]),
        .runtime = atoi(tokens[2]),
        .resources = (resource_request) {
            .memory = atoi(tokens[3]),
            .num_printers = atoi(tokens[4]),
            .num_scanners = atoi(tokens[5]),
            .num_modems = atoi(tokens[6]),
            .num_cds = atoi(tokens[7]),
        },
        .pid = (pid_t)(0),
        .address = -1,
    };

    return p;

} 

void print_process(proc p) {
    printf("PID: %d Priority: %d\n", p.pid, p.priority);
}

bool equals(proc p1, proc p2) {
    return p1.address == p2.address &&
            p1.arrival_time == p2.arrival_time &&
            p1.pid == p2.pid && 
            p1.priority == p2.priority &&
            p1.runtime == p2.runtime &&
            p1.resources.memory == p2.resources.memory &&
            p1.resources.num_cds == p2.resources.num_cds &&
            p1.resources.num_modems == p2.resources.num_modems &&
            p1.resources.num_printers == p2.resources.num_printers &&
            p1.resources.num_scanners == p2.resources.num_scanners;
}