#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>

void load_and_run_elf(char** exe);
void loader_cleanup();
