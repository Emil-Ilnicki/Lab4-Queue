#include "queue.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void push(queue** head, proc process) {

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

proc pop(queue** head) {

    queue* q = (*head);
    if (q == NULL) {
        fprintf(stderr, "Attempted to pop null queue\n");
        exit(1);
    }
    
    proc p = q->process;

    *head = q->next;
    q->next=NULL;

    return p;

}


proc new_process(char* line) {
    char* token = strtok(line, ", ");
    char* tokens[8];
    int i = 0;
    while( token != NULL ) {
        tokens[i++] = token;
        token = strtok(NULL, ", ");
    }

    return (proc) {
        .arrival_time = atoi(tokens[0]),
        .priority = atoi(tokens[1]),
        .runtime = atoi(tokens[2]),
        .resources = (resource_request) {
            .memory = atoi(tokens[3]),
            .num_printers = atoi(tokens[4]),
            .num_scanners = atoi(tokens[5]),
            .num_modems = atoi(tokens[6]),
            .num_cds = atoi(tokens[7]),
            .mem_ptr = -1
        },
        .pid = (pid_t)(0),
    };

} 