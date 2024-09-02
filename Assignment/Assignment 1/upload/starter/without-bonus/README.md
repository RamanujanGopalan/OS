We had beeen given Ehdr struct and Phdr struct in which we had to read the respective file contents.
load_and_run_elf:
  1)Firstly we created a filedescriptor using which we opened the exe file of fib.
  2)Using the opened file we read the contents of elf header into ehdr.
  3)From the ehdr we read program header offset,number of program headers and phentsize and the enrty point.
  4)Using lseek we navigated to the phoff set and iterated through it to find a load type.
  5)If load type segment contained the entry point we loaded into the physical memory using mmap.
  6)We then found the entry point address in the physical memory by using the offset between the loaded segment's offset and the virtual entry.
  7)We then typecasted the entry address to a fuction pointer for _start.
