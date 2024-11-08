
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
    printf("Count = %d\n", count);
    printf("Fragmentation = %d\n", fragmentation);

    return 0;
}
