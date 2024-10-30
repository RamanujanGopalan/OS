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

Queue completed;

void queue(Queue *q) {
    q->first = NULL;
    q->last = NULL;
    q->size = 0;
}

int empty(Queue *q) {
    return q->first == NULL;
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
    new_p->command[cnt] = NULL;

    if(strcmp(tempargs[0],"submit")){
        printf("COMMAND SHOULD START WITH submit\n");
        return;
    }

    if (empty(q)) {
        q->first = new_p;
        q->last = new_p;
    } else {
        q->last->next = new_p;
        q->last = new_p;
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

void my_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Terminating the program .\n");
        int size3 = size(&completed);
        while(size3 > 0) {
            Process* tmp = dequeue(&completed);
            printf("Command: ");
            for (int i = 0; tmp->command[i] != NULL; i++) {
                printf("%s ", tmp->command[i]);
            }
            printf("\n\tPID: %d\n", tmp->pid);
            printf("\tCompletion Time: %ld seconds\n", tmp->completion_time.tv_sec-prog_start.tv_sec-4);
            printf("\tWait Time: %.2lf seconds\n", tmp->wait_time);
            size3--;
        }
        exit(0);
    }
}


int main(int args, char* argv[]) {

    clock_gettime(CLOCK_REALTIME, &prog_start);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Scheduler Launched with %d cpus and %d time quanta\n", atoi(argv[1]), atoi(argv[2]));

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    Queue q;
    queue(&q);
    
    Queue running;
    queue(&running);

    queue(&completed);

    // int count = 0;

    while (1)
    {
        // if (count++%1000000000 != 0){
        //     continue;
        // }
        char command[100];
        ssize_t bytes_read = read(STDIN_FILENO, command, sizeof(command));
        // printf("ASDFGH\n");
        // printf("BYTES READ %d == %d \n", bytes_read, sizeof(Process*));

        if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // "No data written yet. Doing something else...
            // char buff[] = "No data Written\n";
            // write(STDOUT_FILENO, buff, sizeof(buff));
            // printf("asdfg\n");

            // printf("Queue size %d\n", size(&q));
            if (size(&q) == 0 && size(&running) == 0){
                raise(SIGSTOP);
                continue;
            }

            // printf("Varun chomu\n");

            int size1=size(&q);
            int min1 = min(atoi(argv[1]),size1);
            int size2 = 0;
            // printf("Queue size = %d\n", size1);
            // display(&q);

            for(int i=0;i<min1;i++){
                // print("IN FOR");
                Process* p = dequeue(&q);

                struct timespec now;
                clock_gettime(CLOCK_REALTIME, &now);
                p->wait_time += (now.tv_sec - p->eq_time.tv_sec);  //<- Update accumulated wait time

                size1--;
                // printf("dequeued pid %d\n", p->pid);
                if (p->pid == -1){
                    pid_t pid = fork();
                    if (pid == 0){
                        //Child
                        // for (int i = 0; p->command[i] != NULL; i++) {
                        //     printf("arg[%d]: %s\n", i, p->command[i]);
                        // }
                        // printf("CREATED EXECVP CHILD %s\n", p->command[0]);
                        execvp(p->command[0],p->command);
                        perror("execvp failed\n");
                        exit(-5);
                    }
                    else if (pid > 0){
                        //Parent
                        // wait(NULL);
                        
                        p->pid=pid;
                    }
                }
                else if(p->pid!=-1){
                    kill(p->pid,SIGCONT);
                }
                // printf("%d %s equeued 1\n", p->pid,p->command[0]);
                enqueue(&running,p);
                size2++;
            }

            // printf("SLLEPING\n");
            sleep(atoi(argv[2]));

            size2=size(&running);
            int min2 = min(atoi(argv[1]),size2);
            for(int j=0;j<min2;j++){
                int status;
                Process *p=dequeue(&running);
                clock_gettime(CLOCK_REALTIME, &p->dq_time);
                size2--;
                pid_t result = waitpid(p->pid, &status, WNOHANG);
                // printf("%d %s result = %d\n", p->pid,p->command[0], result);
                if(result==0){
                    kill(p->pid,SIGSTOP);
                    // printf("%d %s equeued 2\n", p->pid,p->command[0]);
                    enqueue(&q,p);
                    size1++;
                    // printf("KILLED AND ENQUEUED\n");
                }
                else if(result==p->pid){
                    clock_gettime(CLOCK_REALTIME, &p->completion_time);
                    enqueue(&completed, p);
                    // printf("Compeleted pid %d\n", p->pid);
                    p->completed = 1;
                    // continue;
                }
                else if (result == -1){
                    // printf("completed %d\n", p->completed);
                    if (p->completed != 1){
                        p->completed = 1;
                        // printf("%d %s equeued 3\n", p->pid,p->command[0]);
                        enqueue(&q,p);
                        size1++;
                    }
                }
            }
        }

        else if (bytes_read == -1) {
            perror("Error in reading");
            exit(EXIT_FAILURE);
        }

        else{
            // printf("ASDFGHJ\n");
            // Data received, handle it
            // printf("%d\n", bytes_read);
            // printf("READ %s\n", command);
            enqueue_new(&q, command);
        }
    }

    return 0;
}
