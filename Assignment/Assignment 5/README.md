# Smart Loader

### Group Members
- Ramanujan Gopalan (2023424)
- Varun Golcha (2023584)

### Making the Simple Multithreader
- For implementation of both normal for loop and nested for loop we began by taking the low and high indexes and dividing the problem spaces into chunks based on the num threads given by the user
- We then create the required num of threads passing the required parameters of high and low values using structs containg these values along with the function to be executed
- Then finally, in each thread we run either one for loop or nested for loops iteratin from the low to high values which were passed as a part of the struct

### Contributions
- Ramanujan Gopalan - Debugging, Coding
- Varun Golcha - Coding, Debugging

### GIT Repository
https://github.com/RamanujanGopalan/OS/tree/main/Assignment/Assignment%205
