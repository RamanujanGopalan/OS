#include "loader.h"

Elf32_Ehdr *ehdr = NULL;  
Elf32_Phdr *phdr = NULL; 
void* mapped_pages[50];
int fd = -1;             
int page_count = 0;           
int fragmentation = 0; 

void loader_cleanup() {
    if (ehdr != NULL) {
        free(ehdr);
        ehdr = NULL;
    }
    if (phdr != NULL) {
        free(phdr);
        phdr = NULL;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    for (int i = 0; i<page_count; i++){
        munmap(mapped_pages[i], 4096);
    }
}
void my_handler(int signo, siginfo_t *info, void *context) {
    if (signo == SIGSEGV) {
        uintptr_t seg_address = (uintptr_t)info->si_addr;
        for (int i = 0; i < ehdr->e_phnum; i++) {
            if (phdr[i].p_type == 1){
                if (seg_address >= phdr[i].p_vaddr && seg_address < phdr[i].p_vaddr + phdr[i].p_memsz) {

                    int page_num = (seg_address - phdr[i].p_vaddr) / 4096;
                    void *page = mmap((void *)(phdr[i].p_vaddr + page_num * 4096), 4096,
                                    PROT_READ | PROT_WRITE | PROT_EXEC,
                                    MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
                    if (page == MAP_FAILED) {
                        perror("mmap failed for page");
                        exit(EXIT_FAILURE);
                    }

                    mapped_pages[page_count] = page;

                    page_count++;
                    int to_read = 4096;
                    if (phdr[i].p_vaddr + phdr[i].p_memsz - seg_address < 4096){
                        to_read = phdr[i].p_vaddr + phdr[i].p_memsz - seg_address;
                        fragmentation += 4096-to_read;
                    }

                    lseek(fd, phdr[i].p_offset + page_num * 4096, SEEK_SET);
                    int bytes_read = read(fd, page, to_read);

                    if (bytes_read == -1) {
                        perror("Error reading segment page");
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
            }
        }
    }
}

void load_and_run_elf(char** exe) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = my_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    fd = open(exe[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open ELF file");
        exit(EXIT_FAILURE);
    }

    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error reading ELF header");
        exit(EXIT_FAILURE);
    }

    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
    lseek(fd, ehdr->e_phoff, SEEK_SET);
    if (read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum) != ehdr->e_phentsize * ehdr->e_phnum) {
        perror("Error reading program headers");
        exit(EXIT_FAILURE);
    }
    
    int (*_start)() = (int (*)())ehdr->e_entry;
    int result = _start();
    printf("User _start return value = %d\n", result);
}
int main(int argc, char** argv) {
    if (argc != 2) {
      fprintf(stderr, "Usage: %s <ELF Executable>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    load_and_run_elf(argv);
    loader_cleanup();
    printf("No of page faults = %d\n", page_count);
    printf("No of pages allocated = %d\n", page_count);
    float fragmentation_in_kb = (float)(fragmentation)/1024;
    printf("Fragmentation = %fKB\n", fragmentation_in_kb);

    return 0;
}
