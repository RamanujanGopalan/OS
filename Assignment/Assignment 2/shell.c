#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include<time.h>
void my_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Terminating the program.\n");
        exit(0);
    }
}
void shell_loop() {
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
        printf("ShivamSaanp:~$ ");
        char *command = NULL;
        size_t len = 0;
        if (getline(&command, &len, stdin) == -1) {
            perror("Incorrect Input");
            free(command);
            continue;
        }
        if(!strcmp(command,"\n")){
            free(command);
            continue;
        }
        if (!strcmp(command, "")) {
            free(command); 
            continue;
        }
        status = launch(command);
        // printf("SHELL LOOPED\n");
    } while (status);
}

int launch(char* command){
    int status;
    status = create_process(command);
    return status;
}
int create_process(char* command) {
    if (fork() == 0) {
        int pipes = 0;
        char *tempargs[50];
        int count = 0;
        char *s = (char *)malloc(100 * sizeof(char));
        int h = 0;
        int pipe_pos[50];
        int flag=0;
        for(int i = 0; i <= strlen(command); i++) {
            if (command[i] == ' ' || command[i] == '|' || command[i] == '\0') {
                if(flag){
                s[h] = '\0';
                tempargs[count] = strdup(s);
                count++;
                h = 0;
                if (command[i] == '|') {
                    pipe_pos[pipes]=count;
                    pipes++;
                }
                flag=0;
                }
            } else if (command[i] == '\n') {
                continue;
            } else {
                s[h] = command[i];
                flag=1;
                h++;
            }
        }
        tempargs[count] = NULL;
        time_t t = time(NULL);
        struct tm timer_start = *localtime(&t);
        
        if (pipes == 0) {
            execvp(tempargs[0], tempargs);
            perror("execv failed");
            exit(0);
        }
        else {
            int iter = 0;
            int og_pipes = pipes;
            int fd[2], prev_fd[2];
            int arg_count;
            arg_count = pipe_pos[iter] + 1;
            char *buff[arg_count + 1];
            memset(buff, 0, sizeof(buff));

            for (int i = 0; i < arg_count - 1; i++) {
                buff[i] = tempargs[i];
            }
            buff[arg_count] = NULL;
            pipe(fd);

            if (fork() == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                execvp(buff[0], buff);
                perror("execvp failed");
                exit(1);
            }
            else {
                wait(NULL);
                close(fd[1]);
                prev_fd[0] = fd[0];
            }

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
                    dup2(prev_fd[0], STDIN_FILENO);
                    close(prev_fd[0]);

                    if (pipes > 1) {
                        close(fd[0]);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                    }
                    execvp(buff[0], buff);
                    perror("execvp failed");
                    exit(1);
                }
                else {
                    wait(NULL);
                    close(prev_fd[0]);
                    if (pipes > 1) {
                        close(fd[1]);
                        prev_fd[0] = fd[0];
                    }
                }

                iter++;
                pipes--;
            }
        }
    }
    else{wait(NULL);}
    return 1;
}


int main() {
    shell_loop(); 
    return 0;
}
