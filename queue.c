// Include local implementation of queue structure
#include "queue.h"

// include standard libraries
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * push() Queue Method
 * - Adds an element to the queue
 *
 * @param head The head of the queue
 * @param process The process to add to the queue
 */
void push(queue** head, proc* process) {
    // Create a new node and set it equal to the process
    queue* next_node = malloc(sizeof(queue));
    *next_node = (queue) {
        .process=process,
        .next = NULL,
    };

    // Add it to the queue in order
    if (*head == NULL) {
        *head = next_node;
    }
    else {
        // Iterate to the end of the queue and then add it
        queue* current = *head;
        while (current->next != NULL){
            current = current->next;
        }
        current->next = next_node;
    }
}

/**
 * pop() Method
 * - Remove the first process from the queue
 *
 * @param head
 * @return
 */
proc* pop(queue** head) {
    // Check if the queue is empty
    queue* q = (*head);
    if (q == NULL) {
        fprintf(stderr, "Attempted to pop null queue\n");
        exit(1);
    }

    // Remove the first process
    proc* p = q->process;
    *head = q->next;
    free(q);

    // Return the popped process
    return p;
}

/**
 * new_process() Method:
 * - Returns a new process
 *
 * @param line The input line of the process
 * @return The nrely-created process
 */
proc* new_process(char* line) {
    // Process the line to get the data from it
    char* token = strtok(line, ", ");
    char* tokens[8];
    int i = 0;
    while( token != NULL ) {
        tokens[i++] = token;
        token = strtok(NULL, ", ");
    }

    // Create a new process structure and allocate it the values
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

    // Return that process
    return p;
} 

/**
 * print_process() Method
 * - Prints out the entire process
 *
 * @param p Process to be printed
 */
void print_process(proc p) {
    printf("PID: %d Priority: %d\n", p.pid, p.priority);
}

/**
 * bequals() Method:
 * - Checks if 2 processes are equal or not.
 *
 * @param p1 Process 1
 * @param p2 Process 2
 * @return True if equal, else false
 */
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