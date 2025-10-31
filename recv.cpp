/*
receiver
*/
#include <iostream> // input / output
#include <fstream>  // read and write file
#include <string.h>
#include <unistd.h> // for program to sleep 
#include <sys/mman.h>   // memory management declarations
#include <sys/wait.h>   // for wait, p much unneeded
#include <sys/stat.h>   // For mode constants
#include <fcntl.h>  // For O_* constants

using namespace std;

void recvFile(int sigNum) {
    if (sigNum == SIGUSR1) {
        // allocate shared memory segment and get its id
        int shmid = shm_open("/cpsc351sharedmem", O_RDWR, 0600);

        // check to make sure shared memory was allocated
        if (shmid < 0) {
            perror("failed to allocate mem");
            exit(1);
        }

        // get size of shared memory
        int shared_mem_size = 4096;

        // get pointer to shared memory id
        void* ptr = mmap(0, shared_mem_size, PROT_READ | PROT_WRITE ,MAP_SHARED, shmid, 0);
        
        // check to see if map failed
        if (ptr == MAP_FAILED) {
            perror("mapping failed");
            exit(1);
        }

        // copy data as array into shared memory segment
        //#define buffer 100
        char shm_buffer[100];
        FILE* file = fopen("file_recv", "w");
        strncpy((char*)shm_buffer, (char*)ptr, 100);

        // write the read data to a file called file_recv
        fprintf(file, "received: %s", shm_buffer);
        
        // deallocate the shared memory segment
        unlink("/cpsc351sharedmem");
        fclose(file);
        //delete shm_buffer;

    } else {
        // shared memory segment does not exist
        perror("Missing shared memory segment!\n");
        exit(1);
    }
}

int main() {
    // overrides signal handler for SIGUSR1
    signal(SIGUSR1, recvFile);

    // sleep loop until signal arrives
    while (true) {
        sleep(1);
    }

    return 0;
}