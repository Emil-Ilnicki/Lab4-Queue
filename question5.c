#define _POSIX_SOURCE
#include<stddef.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>

typedef struct {
    char* name;
    int priority;
    pid_t pid;
    int runtime;
} proc;

typedef struct queue {
    proc process;
    struct queue* next;
} queue;


proc split(char* line) {
    char* token = strtok(line, ", ");
    char* tokens[4];
    int i = 0;
    while( token != NULL ) {
        tokens[i++] = token;
        token = strtok(NULL, ", ");
    }

    char* name = malloc(sizeof(char)*256);
    strcpy(name, tokens[0]);
    return (proc) {
        .name = name,
        .priority = atoi(tokens[1]),
        .pid = (pid_t)(0),
        .runtime = atoi(tokens[2]),
    };

} 

queue* push(queue* head, proc process) {

    queue* next_node = malloc(sizeof(queue));
    *next_node = (queue) {
            .process=process,
            .next = NULL,
    };

    if (head == NULL) {
        head = next_node;
    } else {

        queue* current = head;
        while (current->next != NULL){
            current = current->next;
        }

        current->next = next_node;
        
    }

    return head;
}

proc* pop(queue** head) {


    queue* q = (*head);
    if (q == NULL) {
        return NULL;
    }
    
    proc* p = malloc(sizeof(proc));
    p = &q->process;

    *head = q->next;
    q->next=NULL;

    return p;

}

proc* delete_name(queue** head, char* name) {

    queue* curr = *head;
    queue* prev = NULL;

    while (curr != NULL) {
        if (strcmp(curr->process.name, name) == 0) {

            proc* p = malloc(sizeof(proc));
            p = &curr->process;
           
           if (prev != NULL) {
               prev->next = curr->next;
           } else {
               *head = curr->next;
           }
            curr->next = NULL;
           return p;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;

}

proc* delete_pid(queue** head, int pid) {

    queue* curr = *head;
    queue* prev = NULL;

    while (curr != NULL) {
        if (curr->process.pid == pid) {

            proc* p = malloc(sizeof(proc));
            p = &curr->process;
           
           if (prev != NULL) {
               prev->next = curr->next;
           } else {
               *head = curr->next;
           }
            curr->next = NULL;
           return p;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

int main() {

    queue* head = NULL;

    FILE* f = fopen("processes_q5.txt", "r");
    if (f == NULL) {
        perror("processes.txt");
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {

        head = push(head, split(line));
    }

    queue* curr = head;


    while(curr != NULL) {
        if (curr->process.priority == 0) {
            proc* p = delete_name(&head, curr->process.name);
            pid_t pid = fork();

            if (pid == 0) {
                execl("./process", "");
            } else {
                p->pid = pid;
                sleep(p->runtime);
                kill(pid, SIGINT);
                waitpid(pid, NULL, 0);
                printf("%s %d %d %d\n", p->name, p->priority, p->pid, p->runtime);
            }
        }

        curr = curr->next;
    } 

    while(head != NULL) {
        proc* p = pop(&head);
        pid_t pid = fork();

        if (pid == 0) {
            execl("./process", "");
        } else {
            p->pid = pid;
            sleep(p->runtime);
            kill(pid, SIGINT);
            waitpid(pid, NULL, 0);
            printf("%s %d %d %d\n", p->name, p->priority, p->pid, p->runtime);
        }
    }


    return 0;
}