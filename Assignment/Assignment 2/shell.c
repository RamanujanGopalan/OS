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

void my_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Terminating the program.\n");
        complete_history();
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

    do{
        printf("SabhMoMayaHai:~$ ");
        char *command = NULL;
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
        command_lst[command_p++] = command;
        clock_gettime(CLOCK_MONOTONIC, &start_time[time_p]);
        status = launch(command);
        clock_gettime(CLOCK_MONOTONIC, &end_time[time_p]);
        time_p++;
        pid_lst[pid_p++] = status;
        global_p++;
    } while (status);
}

int launch(char* command){
    int status;
    status = create_process(command);
    return status;
}

int create_process(char* command) {
    //CHILD
    if (fork() == 0) {
        int pipes = 0;
        char *tempargs[50];
        int count = 0;
        char *s = (char *)malloc(100 * sizeof(char));
        int h = 0;
        int pipe_pos[50];
        int i = 0;

        //PARSING
        int wspace_flag = 0;
        for(int i = 0; i <= strlen(command); i++) {
            if (command[i] == ' ' || command[i] == '|' || command[i] == '\0') {
                s[h] = '\0';
                tempargs[count] = strdup(s);
                count++;
                h = 0;
                if (command[i] == '|') {
                    pipe_pos[pipes]=count;
                    pipes++;
                }
            }
            else if (command[i] == '\n') {
                continue;
            }
            else {
                s[h] = command[i];
                h++;
            }
        }
        tempargs[count] = NULL;

        if (pipes == 0) {
            execvp(tempargs[0], tempargs);
            perror("execv failed");
            exit(0);
        }

        //HANDLING PIPES
        else {
            int iter = 0;
            int og_pipes = pipes;
            
            int arg_count;
            arg_count = pipe_pos[iter] + 1;
            char *buff[arg_count + 1];
            memset(buff, 0, sizeof(buff));
            for (int i = 0; i < arg_count - 1; i++) {
                buff[i] = tempargs[i];
            }
            buff[arg_count] = NULL;

            int fd[2], prev_fd;
            pipe(fd);

            //RUNNING 1st COMMAND
            if (fork() == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                execvp(buff[0], buff);
                perror("execvp failed");
                exit(1);
            }
            else {
                wait(NULL);
                close(fd[1]);
                prev_fd = fd[0];
            }

            // RUNNING REST OF THE COMMANDS
            iter = 1;
            while (pipes > 0) {
                if (iter == og_pipes) {
                    arg_count = count - pipe_pos[iter - 1] + 1;
                }
                else {
                    arg_count = pipe_pos[iter] - pipe_pos[iter - 1] + 1;
                }

                char *buff[arg_count + 1];
                memset(buff, 0, sizeof(buff));
                for (int i = 0; i < arg_count - 1; i++) {
                    buff[i] = tempargs[pipe_pos[iter - 1] + i];
                }
                buff[arg_count] = NULL;

                if (pipes > 1) {
                    pipe(fd);
                }

                if (fork() == 0) {
                    dup2(prev_fd, STDIN_FILENO);
                    if (pipes > 1) {
                        close(fd[0]);
                        dup2(fd[1], STDOUT_FILENO);
                    }
                    execvp(buff[0], buff);
                    perror("execvp failed");
                    exit(1);
                }
                else {
                    wait(NULL);
                    close(prev_fd);
                    if (pipes > 1) {
                        close(fd[1]);
                        prev_fd = fd[0];
                    }
                }
                iter++;
                pipes--;
            }
        }
        exit(0);
    }
    else{
        int status = wait(NULL);
        return status;
    }
    return 1;
}


int main() {
    shell_loop(); 
    return 0;
}
