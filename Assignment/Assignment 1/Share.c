#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  
}

/*
 * Load and run the ELF executable file
 */

void load_and_run_elf(char** exe) {
  fd = open(exe, O_RDONLY);
  // 1. Load entire binary content into the memory from the ELF file.
  if(fd<0){
    printf("ERROR");
    exit(1);
  }
  Elf32_Ehdr ehdr;
  if(read(fd,&ehdr,sizeof(ehdr))!=sizeof(ehdr)){
    printf("ERROR");
    exit(1);
  }
  // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  unsigned int phoff=ehdr.e_phoff;
  unsigned short phentsize=ehdr.e_phentsize;
  unsigned int phnum=ehdr.e_phnum;
  unsigned int entry=ehdr.e_entry;
  Elf32_Phdr Phdr;

  for(int i=0;i<phnum;i++){
    lseek(fd,phoff+phentsize*i,SEEK_SET);
    if(read(fd,&Phdr,sizeof(Phdr))!=sizeof(Phdr)){
      printf("ERROR");
      exit(1);
    }
    if(Phdr.p_type!=1){
      continue;
    }
    else if(Phdr.p_vaddr<=entry && Phdr.p_vaddr+Phdr.p_memsz>=entry){
      break;
    }
  }
  
  int realoff=entry-Phdr.p_offset;
  
  // 3. Allocate memory of the size "p_memsz" using mmap function
  //    and then copy the segment content
  void* virtualmem=mmap(NULL,Phdr.p_memsz,PROT_READ|PROT_EXEC|PROT_WRITE,MAP_PRIVATE,0,0);
  
  lseek(fd, Phdr.p_offset, SEEK_SET);

  if (read(fd, &virtualmem, Phdr.p_memsz) != Phdr.p_memsz){
    printf("ERROR");
    exit(1);
  }
  
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  void* real_entry=realoff+virtualmem;
  
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)(void) = (int (*)(void)) real_entry;
  // 6. Call the "_start" method and print the value returned from the "_start"
  
  int result = _start();
  printf("User _start return value = %d\n",result);
}
