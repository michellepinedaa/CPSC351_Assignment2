#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

static constexpr const char* QUEUE_NAME = "/cpsc351queue";
static constexpr size_t MAX_SIZE   = 4096;
static constexpr const char* MSG_STOP   = "STOP";

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;

    int out_fd = open("file_recv", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("open file_recv");
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        exit(1);
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        bytes_read = mq_receive(mq, buffer, MAX_SIZE, nullptr);
        if (bytes_read < 0) {
            perror("mq_receive");
            break;
        }

        if (bytes_read == 0 || strncmp(buffer, MSG_STOP, 4) == 0) {
            break;
        }

        if (write(out_fd, buffer, bytes_read) == -1) {
            perror("write");
            break;
        }
    }

    close(out_fd);
    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return 0;
}

