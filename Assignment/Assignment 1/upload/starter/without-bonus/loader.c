#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  printf("GHJ\n");
  if(ehdr!=NULL){free(ehdr);}
  printf("JHBFD\n");
  if(fd>=0){free(fd);}
  printf("JBKFB\n");
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {

  fd = open(exe, O_RDONLY);

  // 1. Load entire binary content into the memory from the ELF file.
  if(fd<0){
    printf("ERROR 1\n");
    exit(1);
  }

  ehdr=(Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  if(read(fd,ehdr,sizeof(Elf32_Ehdr))!=sizeof(Elf32_Ehdr)){
    printf("ERROR 2\n");
    exit(1);
  }

  unsigned int phoff=ehdr->e_phoff;
  unsigned short phentsize=ehdr->e_phentsize;
  unsigned int phnum=ehdr->e_phnum;
  unsigned int entry=ehdr->e_entry;

  // 2. Iterate through the phdr table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
  int flag=0;
  for(int i=0;i<phnum;i++){
    lseek(fd,phoff+phentsize*i,SEEK_SET);
    if(read(fd,phdr,sizeof(Elf32_Phdr))!=sizeof(Elf32_Phdr)){
      printf("ERROR 3\n");
      exit(1);
    }
    if(phdr->p_type!=1){
      continue;
    }
    else if(phdr->p_vaddr<=entry && phdr->p_vaddr+phdr->p_memsz>=entry){
      flag=1;
      break;
    }
  }
  if(!flag){
    printf("ERROR 4\n");
    exit(1);
  }
  int realoff=entry-phdr->p_vaddr;

  // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
  void* virtual_mem=mmap(NULL,phdr->p_memsz,PROT_READ|PROT_EXEC|PROT_WRITE,MAP_PRIVATE,fd,phdr->p_offset);
  
  lseek(fd, phdr->p_offset, SEEK_SET);

  if (read(fd, virtual_mem, phdr->p_memsz) != phdr->p_memsz){
    perror("ERROR 5\n");
    exit(1);
  }

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  void* real_entry=realoff+virtual_mem;

  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)() = (int (*)()) real_entry;

  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
  munmap(virtual_mem,phdr->p_memsz);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
