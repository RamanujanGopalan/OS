#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

static struct timespec prog_start;

int min(int x, int y){
    if (x<=y){return x;}
    return y;
}

typedef struct Process{
    int completed;
    pid_t pid;
    int num_commands;
    char* command[50];

    struct timespec completion_time;
    struct timespec dq_time;
    struct timespec eq_time;
    double wait_time;
    struct Process* next;
}Process;


typedef struct {
    Process *first;
    Process *last;
    int size;
} Queue;

void queue(Queue *q) {
    q->first = NULL;
    q->last = NULL;
    q->size = 0;
}

int empty(Queue *q) {
    return q->first == NULL;
}

void enqueue_new(Queue *q, char *command) {
    Process* new_p = malloc(sizeof(Process));
    new_p->pid = -1;
    new_p->completed = 0;
    new_p->next = NULL;
    new_p->wait_time = 0;
    clock_gettime(CLOCK_REALTIME, &new_p->eq_time);

    int pipes = 0;
    char *tempargs[50];
    int count = 0;
    char *s = (char *)malloc(100 * sizeof(char));
    int h = 0;
    int pipe_pos[50];

    int ws_flag = 0;

    for (int i = 0; i <= strlen(command); i++) {
        // printf("s = %s\n", s);
        if (command[i] == '\n'){
            tempargs[count] = strdup(s);
            count++;
            break;
        }
        if (command[i] == ' ' || command[i] == '|' || command[i] == '\0') {
            if (ws_flag == 1) {
                s[h] = '\0';
                tempargs[count] = strdup(s);
                free(s);
                s = (char *)calloc('\0',100 * sizeof(char));
                count++;
                h = 0;
                ws_flag = 0;
            }
            if (command[i] == '|') {
                pipe_pos[pipes] = count;
                pipes++;
            }
        } else if (command[i] != '\n') {
            s[h] = command[i];
            h++;
            ws_flag = 1;
        }
    }
    tempargs[count] = NULL;
    free(s);

    // printf(tempargs[1]);

    int cnt = 1;
    while (tempargs[cnt] != NULL) {
        new_p->command[cnt-1] = tempargs[cnt];
        cnt++;
    }
    printf("\n");
    new_p->command[cnt] = NULL;


    if (empty(q)) {
        q->first = new_p;
        q->last = new_p;
    } else {
        q->last->next = new_p;
        q->last = new_p;
    }
    q->size++;
}

void enqueue(Queue* q, Process* p) {
    clock_gettime(CLOCK_REALTIME, &p->eq_time);
    p->next = NULL;
    if (empty(q)) {
        q->first = p;
        q->last = p;
    } else {
        q->last->next = p;
        q->last = p;
    }
    q->size++;
}

Process* dequeue(Queue *q) {
    if (empty(q)) {
        printf("QUEUE IS EMPTY\n");
        return NULL;
    }
    Process* temp = q->first;
    q->first = q->first->next;
    if (q->first == NULL) {
        q->last = NULL;
    }
    temp->next = NULL;
    q->size--;
    return temp;
}

void display(Queue *q) {
    if (empty(q)) {
        printf("EMPTY\n");
        return;
    }
    Process* temp = q->first;
    printf("QUEUE --> ");
    while (temp != NULL) {
        printf(" %d ->", temp->pid);
        temp = temp->next;
    }
    printf("\n");
}

int size(Queue *q){
    return q->size;
}
