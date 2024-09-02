Group Members:
Ramanujan Gopalan(2023424)
Varun Golcha(2023584)

Given an ELF executable file we had to load the file onto the physical memory and execute it

load_and_run_elf():
  1)Firstly we created a filedescriptor using which we opened the exe file of fib.\n
  2)Using the opened file we read the contents of elf header into ehdr.
  3)From the ehdr we read program header offset,number of program headers and phentsize and the enrty point.
  4)Using lseek we navigated to the phoff set and iterated through it to find a load type.
  5)If load type segment contained the entry point we loaded into the physical memory using mmap.
  6)We then found the entry point address in the physical memory by using the offset between the loaded segment's offset and the virtual entry.
  7)We then typecasted the entry address to a fuction pointer for _start and printed the result.
  
loader_cleanup():
  Freed the allocated memory for the pointer ehdr and phdr and closed the file
  
Error Handling:
  1)Unable to read ELF Header/Program Header
  2)Entry point not found
  3)exe file not provided

GitHub Link:
https://github.com/RamanujanGopalan/OS/tree/main/Assignment/Assignment%201/upload/starter/without-bonus

Contributions:
  Coding - Varun Golcha, Ramanujan Gopalan
  Debugging - Ramanujan Gopalan, Varun Golcha
