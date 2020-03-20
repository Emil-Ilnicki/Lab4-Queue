// Standard Libraries to include
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

// Local structure implementations to include
#include "hostd.h"

// Global variable to count the number of ticks
unsigned int ticks = 0;

// Define a simple structure to hold the different type fo resources
const resources resources_default = {
    .avail_printers = MAX_PRINTERS,
    .avail_scanners = MAX_SCANNERS,
    .avail_modems = MAX_MODEMS,
    .avail_cds = MAX_CDS,
};

/**
 * request_resources() Method:
 * - The function that allocates resources
 *
 * @param avail_resources The available resources as a pointer to a resources object
 * @param request a request structure parameter object
 * @param realtime parameter to tell if this is realtime or not
 * @return The starting address of the memory block allocated
 */
int request_resources(resources* avail_resources, resource_request request, bool realtime) {
    // Check if the resources requested exceed the maximum available per resource
    if (request.num_scanners > avail_resources->avail_scanners &&
        request.num_printers > avail_resources->avail_printers &&
        request.num_modems > avail_resources->avail_modems &&
        request.num_cds > avail_resources->avail_cds) {
            return -1;
        }

    // Define a variable to hold a start address and the last possible address, along with the start block
    int start_address = -1;
    unsigned int last_address = realtime ? MEMORY : MEMORY - 64;
    unsigned int start_block = realtime ? MEMORY - 64 : 0;

    // In a for loop, iterate over every address from the start block up till the last adress
    for (unsigned int i = start_block; i < last_address; i++) {
        // if looking for the start of a hole
        if (avail_resources->avail_mem[i] == 0 && start_address == -1) {
            start_address = i;
        }

        // set the end address to the end of the hole
        if ((avail_resources->avail_mem[i] == 1 || i == last_address - 1 ) && start_address >= 0) {
            // if hole is not large enough, keep searching
            if (i - start_address < request.memory - 1) {
                start_address = -1;
            } else {
                break;
            }
        }
    }

    // if there is no room for the process
    if (start_address == -1) {
        return -1;
    } 

    // In a for-loop, fill up the memory slots
    for (unsigned int i = start_address; i < start_address + request.memory; i++) {
        avail_resources->avail_mem[i] = 1;
    }

    // Remove the memory allocations from the original resource hold by th amount allocated to the passed-in resources.
    request.num_scanners -= avail_resources->avail_scanners;
    request.num_printers -= avail_resources->avail_printers;
    request.num_modems -= avail_resources->avail_modems;
    request.num_cds -= avail_resources->avail_cds;

    // Return the start address
    return start_address;

}

/**
 * free_resources() Method:
 * - Frees the resources of a passed-in procedure
 *
 * @param avail_resources Holds the available resources
 * @param p Holds a process to freed
 */
void free_resources(resources* avail_resources, proc* p) {
    // Iterate over the process's entire memory and free it
    for (unsigned int i = p->address; i < p->address + p->resources.memory ; i++) {
        avail_resources->avail_mem[i] = 0;
    }

    // Add it to the resource tracker structure.
    p->resources.num_scanners += avail_resources->avail_scanners;
    p->resources.num_printers += avail_resources->avail_printers;
    p->resources.num_modems += avail_resources->avail_modems;
    p->resources.num_cds += avail_resources->avail_cds;

    // De-allocate the process
    free(p);
}

/**
 * run_realtime():
 * - The main driver code of the program
 *
 * @param realtime The realtime queue pointer
 * @param avail_res The available resources pointer
 */
void run_realtime(queue** realtime, resources* avail_res) {
    // Basically, while (true)
    while (*realtime != NULL) {
        // Get the procedure from the realtime queue
        // Request resources for it, or print an error message if non availble
        proc* p = pop(realtime);
        int addr = request_resources(avail_res, p->resources, true);
            if (addr < 0) {
                fprintf(stderr, "No memory available for realtime proc\n");
                exit(1);
            }
        p->address = addr;
        pid_t pid = fork();

        // If the pid is negative, then exit with a realtime fork message
        // Else if the pid is 0, then execute the process
        // Else, print the process, sleep for its runtime, then kill it, and free its resource
        if (pid < 0) {
            perror("realtime fork");
            exit(1);
        
        } else if (pid == 0) {
            execl("./process", "");
        } else {
            p->pid = pid;
            print_process(*p);
            sleep(p->runtime);
            kill(pid, SIGINT);
            wait(NULL);
            ticks += p->runtime;
            free_resources(avail_res, p);
        }
    }

}

/**
 * run_processes() method:
 * - A method to run a process
 *
 * @param process A process to include
 * @param avail_resources The available resources as a pointer
 * @param next_queue The next queue to refer to
 */
void run_process(proc* process, resources* avail_resources, queue** next_queue) {
    // If the process has pid of 0, then fork it. If it is negative, then exit with an error.
    // Else if the pid itself is a 0, then execute it
    // If none of the above, then update the process's pid to the new one
    if (process->pid == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("user process fork");
            exit(1);
        }
        else if (pid == 0) {
            execl("./process", "");
        }
        else {
            process->pid = pid;
        }
    }

    // If the process pid is not 0, then kill it.
    else {
        kill(process->pid, SIGCONT);
    }

    // Kill the process
    // sleep for a second
    // increment ticks and decrement the process runtime
    print_process(*process);
    sleep(1);
    ticks++;
    process->runtime--;

    // If the process has no runtime, wait for the process to change its state, then kill it, and free its resources
    if (process->runtime == 0) {
        kill(process->pid, SIGINT);
        waitpid(process->pid, NULL, WUNTRACED);
        free_resources(avail_resources, process);
    }
    // Else kill the process, wait for its state to change, an push it to the next queue
    else {
        kill(process->pid, SIGTSTP);
        
        waitpid(process->pid, NULL, WUNTRACED);
        if (process->priority < 3) {
            process->priority++;
        }
        push(next_queue, process);
    }
}

/**
 * main() Driver code
 * - Used to run program
 *
 * @return Exit state of program
 */
int main() {

    // Declare all resources to be used
    resources avail_resources = resources_default;
    queue* dispatcher = NULL;
    queue* user_queue = NULL;
    queue* realtime = NULL;
    queue* priority_1 = NULL;
    queue* priority_2 = NULL;
    queue* priority_3 = NULL;
    
    // Open the dispatchlist file, or exit with an error if it fails
    FILE* f = fopen("dispatchlist", "r");
    if (f == NULL) {
        perror("dispatchlist");
    }

    // Get the data from the file
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        push(&dispatcher, new_process(line));
    }
    
    int user_queue_size = 0;
    // main dispatcher loop    
    while (1) {

        // get all incoming processes that have arrived
        while (dispatcher != NULL && dispatcher->process->arrival_time <= ticks) {
   
            proc* p = pop(&dispatcher);
            // assign it to the correct queue
            if (p->priority == 0) {
                push(&realtime, p);
            } else {
                // printf("%d", p->resources.memory)
                push(&user_queue, p);
                user_queue_size++;
            }
        }
        
        if (realtime != NULL) {
            // run all realtime processes
            run_realtime(&realtime, &avail_resources);
            continue; // check to see if any new incoming processes have shown up
        }

        // check to see if any of the ready queue are ready to run
        // printf("%d %d\n", user_queue_tail->arrival_time, user_queue_tail->resources.memory);
        int i = 0;
        while(user_queue != NULL) {
            // pop a process from the queue
            proc* p = pop(&user_queue);
            user_queue_size--;
            i++;

            // Request reources for that process
            // If an invalid address returned, then push the process onto the queue, and move to the next iteration
            int addr = request_resources(&avail_resources, p->resources, false);
            if (addr < 0) {
                user_queue_size++;
                push(&user_queue, p);

                if (i >= user_queue_size) {
                    break;
                }
                if (i == 10) {
                    exit(1);
                }
                continue;
            }

            // Based on its priority, push it to the appropriate queue
            p->address = addr;
            switch(p->priority) {
                case 1:
                    push(&priority_1, p);
                    break;
                case 2:
                    push(&priority_2, p);
                    break;
                default:
                    push(&priority_3, p);
                    break;
            }
        }

        // Get the next process based on priority (all 1st priority process, then 2nd priority processes, ...)
        if (priority_1 != NULL) {
            proc* p = pop(&priority_1);
            run_process(p, &avail_resources, &priority_2);
        } else if (priority_2 != NULL) {
            proc* p = pop(&priority_2);
            run_process(p, &avail_resources, &priority_3);
        } else if (priority_3 != NULL) {
            proc* p = pop(&priority_3);
            run_process(p, &avail_resources, &priority_3);
        
        // if all queues are empty, end the dispatcher
        } else {
            break;
        }
    }
}