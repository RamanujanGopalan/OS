#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include<time.h>

char** command_lst[300];
int pid_lst[300];
struct timespec start_time[300];
struct timespec end_time[300];
int global_p = 0;

char* n_cpu;
char* time_quanta;

pid_t scheduler;

void my_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C) in parent. Terminating the program.\n");
        kill(scheduler, SIGCONT);
        kill(scheduler, SIGINT);
        exit(0);
    }
}

void command_history(){
    int i = 0;
    while (i<global_p){
        printf("%d. %s", i, command_lst[i++]);
    }
}

void complete_history(){
    int i = 0;
    while (i<global_p){
        printf("%d. %d\n\t%s\t%lu\n\t%lu\n", i+1, pid_lst[i], command_lst[i], start_time[i], end_time[i].tv_sec-start_time[i].tv_sec);
        i++;
    }
}

void shell_loop() {
    int command_p = 0;
    int pid_p = 0;
    int time_p = 0;

    int status;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_handler;
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0){
        char* argv[] = {"./SimpleScheduler", n_cpu, time_quanta, "\0"};
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execvp(argv[0], argv);
        perror("Scheduler execvp FAILED\n");
        exit(0);
    }
    else if (pid>0){
        scheduler = pid;
        // close(fd[0]);
        status = 1;
        do{
            printf("SabhMoMayaHai:~$ ");
            char* command = NULL;
            size_t len = 0;
            if (getline(&command, &len, stdin) == -1) {
                perror("Incorrect Input");
                free(command);
                continue;
            }
            // printf("%s\n",command);
            if(!strcmp(command,"\n")){
                free(command);
                continue;
            }
            if (!strcmp(command, "")) {
                free(command); 
                continue;
            }
            if (strcmp(command, "history\n") == 0){
                command_history(command_lst);
                continue;
            }

            char *temp = realloc(command, strlen(command) + 1); // +1 for newline, +1 for null terminator
            if (temp == NULL) {
                perror("Memory allocation failed");
                free(command);
                exit(1);
            }
            command = temp;
            strcat(command, "\n");

            if (write(fd[1], command, strlen(command)) != strlen(command)){
                perror("Error in write\n");
                exit(1);
            }
            kill(scheduler, SIGCONT);
            sleep(1);
            // printf("%d\n", strlen(command));
        } while (status);
    }
}

int main(int args, char const *argv[]) {
    if (args != 3){
        perror("INCORRECT INPUT\n");
        exit(1);
    }
    n_cpu = (argv[1]);
    time_quanta = (argv[2]);
    shell_loop(); 
    return 0;
}
