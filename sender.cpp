/*
sender
*/
#include <iostream> // input / output
#include <fstream>  // read and write file
#include <string.h>
#include <sys/mman.h>   // memory management declarations
#include <sys/wait.h>   // for wait
#include <sys/stat.h>   // For mode constants
#include <fcntl.h>  // For O_* constants
#include <sys/shm.h>


using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        perror("must have two arguments: file name and receiver pid");
        exit(1);
    }
    // get receiver pid from argument line
    int pid = atoi(argv[2]);

    // get file name to send
    char *file = argv[1];

    // size of shared memory segment
    const int shared_mem_size = 4096;

    // allocate a shared memory segment
    int shmid = shm_open("/cpsc351sharedmem", O_CREAT | O_RDWR, 0600);

    // set the size of the shared memory segment
    int val = ftruncate(shmid, shared_mem_size);

    // check to make sure valid size
    if (val < 0) {
        perror("truncate failed");
        exit(1);
    }

    // get pointer to shared memory segment
    void* shared_mem_ptr = mmap(0, shared_mem_size, PROT_READ | PROT_WRITE ,MAP_SHARED, shmid, 0);
    
    // check to make sure valid mapping
    if (shared_mem_ptr == MAP_FAILED) {
        perror("mapping failed");
        exit(1);
    }

    // copy data as array into shared memory segment
    strncpy((char*)shared_mem_ptr, file, 100);

    // send SIGUSR1 to receiver
    kill(SIGUSR1, pid);

    return 1;
}