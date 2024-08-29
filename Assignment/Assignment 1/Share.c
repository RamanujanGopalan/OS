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

  // CHOMU -> Entire Binary Content Loaded
  // if(fd>=0){
  //   off_t filesize=lseek(fd,0,SEEK_END);
  //   char* buf=(char*)malloc(filesize);
  //   long long sz;
  //   sz=read(fd,buf,filesize);
  //   printf("Those bytes are as follows: %s\n", buf);
  // }

  Elf32_Ehdr* ehdr = (Elf32_Ehdr*)fd;
  Elf32_Phdr* phEntry = ehdr + ehdr->e_phoff;

  short phSize = ehdr->e_phentsize;
  short phNum = ehdr->e_phnum;


  // 2. Iterate through the PHDR table and find the section of PT_LOAD type that contains the address of the entrypoint method in fib.c
  Elf32_Phdr* phPointer = phEntry;
  short count = 0;
  while (count++<phNum){
    if (phPointer->p_type == 1){
      
    }
  }
  

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  // int result = _start();
  // printf("User _start return value = %d\n",result);
}

// From Launch.c
int main(int argc, char** argv) 
{
  // if(argc != 2) {
  //   printf("Usage: %s <ELF Executable> \n",argv[0]);
  //   exit(1);
  // }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(*argv[1]);
  // 3. invoke the cleanup routine inside the loader
  return 0;
}
