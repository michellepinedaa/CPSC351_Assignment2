#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>

using namespace std;

void recvFile(int sigNum) {
    if (sigNum == SIGUSR1) {
        // open shared memory
        int shmid = shm_open("/cpsc351sharedmem", O_RDWR, 0600);
        if (shmid < 0) {
            perror("shm_open failed");
            exit(1);
        }

        const int SHM_SIZE = 4096;

        void* ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }

        // copy data to file
        char buffer[100];
        strncpy(buffer, static_cast<char*>(ptr), 100);

        ofstream outFile("file_recv");
        outFile << "received: " << buffer;
        outFile.close();

        // cleanup
        munmap(ptr, SHM_SIZE);
        shm_unlink("/cpsc351sharedmem");
    } else {
        cerr << "Wrong signal!" << endl;
        exit(1);
    }
}

int main() {
    signal(SIGUSR1, recvFile);

    cout << "Receiver PID: " << getpid() << endl;
    cout << "Waiting for signal..." << endl;

    while (true) {
        sleep(1);
    }

    return 0;
}
