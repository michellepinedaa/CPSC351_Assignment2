#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <cstdlib>

#define QUEUE_NAME "/cpsc351queue"
#define MAX_SIZE 4096
#define MSG_STOP "STOP"

using namespace std;

int main() {
    // Set message queue attributes
    struct mq_attr attr;
    attr.mq_flags = 0;          // blocking queue
    attr.mq_maxmsg = 10;        // max 10 messages
    attr.mq_msgsize = MAX_SIZE; // max message size
    attr.mq_curmsgs = 0;

    // Create the message queue (or open if exists)
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    char buffer[MAX_SIZE + 1]; // buffer for received message
    ssize_t bytes_read;

    // Open file to write received data
    int out_fd = open("file_recv", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("open file_recv");
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        exit(1);
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // Receive message (blocking)
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, nullptr);
        if (bytes_read < 0) {
            perror("mq_receive");
            break;
        }

        // Check for "empty" message (end of file)
        if (bytes_read == 0 || strncmp(buffer, MSG_STOP, 4) == 0) {
            break;
        }

        // Write received data to file
        if (write(out_fd, buffer, bytes_read) == -1) {
            perror("write");
            break;
        }
    }

    close(out_fd);
    mq_close(mq);
    mq_unlink(QUEUE_NAME); // delete the message queue

    return 0;
}
