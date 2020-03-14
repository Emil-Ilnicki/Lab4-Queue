#include "hostd.h"
#include "queue.h"

#include <unistd.h>
#include <stdio.h>

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
    
    for (unsigned int i = 0; i < last_address; i++) {

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

void free_resources(resources* avail_resources, resource_request loaned_resources, unsigned int address) {

    for (unsigned int i = address; i < address + loaned_resources.memory; i++) {
        avail_resources->avail_mem[i] = 0;
    }

    loaned_resources.num_scanners += avail_resources->avail_scanners;
    loaned_resources.num_printers += avail_resources->avail_printers;
    loaned_resources.num_modems += avail_resources->avail_modems;
    loaned_resources.num_cds += avail_resources->avail_cds;


}


// #include<stddef.h>
// #include<stdlib.h>
// #include<stdio.h>

// #include<unistd.h>
// #include<signal.h>
// #include<sys/types.h>
// #include<sys/wait.h>
// #include<string.h>







// int main() {

//     queue* head = NULL;

//     FILE* f = fopen("processes_q5.txt", "r");
//     if (f == NULL) {
//         perror("processes.txt");
//     }

//     char line[512];
//     while (fgets(line, sizeof(line), f)) {

//         head = push(head, split(line));
//     }

//     queue* curr = head;


//     while(curr != NULL) {
//         if (curr->process.priority == 0) {a
//             proc* p = delete_name(&head, curr->process.name);
//             pid_t pid = fork();

//             if (pid == 0) {
//                 execl("./process", "");
//             } else {
//                 p->pid = pid;
//                 sleep(p->runtime);
//                 kill(pid, SIGINT);
//                 waitpid(pid, NULL, 0);
//                 printf("%s %d %d %d\n", p->name, p->priority, p->pid, p->runtime);
//             }
//         }

//         curr = curr->next;
//     } 

//     while(head != NULL) {
//         proc* p = pop(&head);
//         pid_t pid = fork();

//         if (pid == 0) {
//             execl("./process", "");
//         } else {
//             p->pid = pid;
//             sleep(p->runtime);
//             kill(pid, SIGINT);
//             waitpid(pid, NULL, 0);
//             printf("%s %d %d %d\n", p->name, p->priority, p->pid, p->runtime);
//         }
//     }


//     return 0;
// }

int main() {

    // resources avail_resources = resources_default;
    // queue* realtime = NULL;
    // queue* priority_1 = NULL;
    // queue* priority_2 = NULL;
    // queue* priority_3 = NULL;

    queue* head = NULL;
    FILE* f = fopen("dispatchlist", "r");
    if (f == NULL) {
        perror("dispatchlist");
    }

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        push(&head, new_process(line));
    }

    while (head != NULL) {
        proc p = pop(&head);
        printf("%d\n", p.resources.memory);
    }




}