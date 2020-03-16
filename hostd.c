#include "hostd.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

unsigned int ticks = 0;

const resources resources_default = {
    .avail_printers = MAX_PRINTERS,
    .avail_scanners = MAX_SCANNERS,
    .avail_modems = MAX_MODEMS,
    .avail_cds = MAX_CDS,
};

int request_resources(resources* avail_resources, resource_request request, bool realtime) {
    
    if (request.num_scanners > avail_resources->avail_scanners &&
        request.num_printers > avail_resources->avail_printers &&
        request.num_modems > avail_resources->avail_modems &&
        request.num_cds > avail_resources->avail_cds) {
            return -1;
        }

    int start_address = -1;
    unsigned int last_address = realtime ? MEMORY : MEMORY - 64;

    unsigned int start_block = realtime ? MEMORY - 64 : 0;
    
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

    for (unsigned int i = start_address; i < start_address + request.memory; i++) {
        avail_resources->avail_mem[i] = 1;
    }

    request.num_scanners -= avail_resources->avail_scanners;
    request.num_printers -= avail_resources->avail_printers;
    request.num_modems -= avail_resources->avail_modems;
    request.num_cds -= avail_resources->avail_cds;

    return start_address;

}

void free_resources(resources* avail_resources, proc* p) {

    for (unsigned int i = p->address; i < p->address + p->resources.memory ; i++) {
        avail_resources->avail_mem[i] = 0;
    }

    p->resources.num_scanners += avail_resources->avail_scanners;
    p->resources.num_printers += avail_resources->avail_printers;
    p->resources.num_modems += avail_resources->avail_modems;
    p->resources.num_cds += avail_resources->avail_cds;

    free(p);

}

void run_realtime(queue** realtime, resources* avail_res) {

    while (*realtime != NULL) {
        proc* p = pop(realtime);
        int addr = request_resources(avail_res, p->resources, true);
            if (addr < 0) {
                fprintf(stderr, "No memory available for realtime proc\n");
                exit(1);
            }
        p->address = addr;
        pid_t pid = fork();

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

void run_process(proc* process, resources* avail_resources, queue** next_queue) {

    if (process->pid == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("user process fork");
            exit(1);
        } else if (pid == 0) {
            execl("./process", "");
        } else {
            process->pid = pid;
        }
    } else {
        kill(process->pid, SIGCONT);
    }
    print_process(*process);
    sleep(1);
    ticks++;
    process->runtime--;

    if (process->runtime == 0) {
        kill(process->pid, SIGINT);
        waitpid(process->pid, NULL, WUNTRACED);
        free_resources(avail_resources, process);
    } else {
       
        kill(process->pid, SIGTSTP);
        
        waitpid(process->pid, NULL, WUNTRACED);
        if (process->priority < 3) {
            process->priority++;
        }
        push(next_queue, process);
    }


}

int main() {

    resources avail_resources = resources_default;
    queue* dispatcher = NULL;
    queue* user_queue = NULL;
    queue* realtime = NULL;
    queue* priority_1 = NULL;
    queue* priority_2 = NULL;
    queue* priority_3 = NULL;
    

    FILE* f = fopen("dispatchlist", "r");
    if (f == NULL) {
        perror("dispatchlist");
    }

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
            
            proc* p = pop(&user_queue);
            user_queue_size--;
            i++;

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