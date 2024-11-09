# Smart Loader

### Group Members
- Ramanujan Gopalan (2023424)
- Varun Golcha (2023584)

### Making the Smart Loader
- Building on our previous code from the 1st Assignment, we added a signal handler to handle the Segmentation Faults that would occur when accessing a memory adresswhich is not present in a page which has alredy been loaded to the physical memory
- Once a seg fault occurs the control is transfered to our handler where we get the virtual adress where it occurred
- We then iterate through the phdr table (using info from the ehdr) until we find the segment which the contains the required adress
- We find the appropriate page number within the segment and the page where the required adress is present is loaded on the physical memory using mmap()
- Before reading on to the mmaped adress we check how many bytes are to be read (4096 or <4096) and accordingly read the part of the segment to save on the physical memory
- The control is then returned back to the user code and execution continues as usual until the next seg fault occurs

### Contributions
- Ramanujan Gopalan - Debugging, Coding
- Varun Golcha - Coding, Debugging

### GIT Repository
https://github.com/RamanujanGopalan/OS/tree/main/Assignment/Assignment%204
