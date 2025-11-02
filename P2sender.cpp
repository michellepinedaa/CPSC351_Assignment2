/*
Sender Function

This is meant to send out a message to the receiver code

Build: g++ -std=c++17 -O2 sender.cpp -o sender -lrt

Run: ./sender <file_name>

*/

#include <cstdio> //std::fprintf
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCESS (Same as return 1 or return 0)
#include <cstring> //std::strerror
#include <cerrno> //Access last error code (errno)
#include <fcntl.h> // open()
#include <mqueue.h>  // mqd_t, mq_open, mq_attr
#include <sys/stat.h> // S_IRUSR, S_IWUSR
#include <unistd.h> //read(), write(), close()

static constexpr const char* QUEUE_NAME = "/cpsc351messagequeue"; // POSIX requires leading '/'
static constexpr size_t CHUNK_SIZE = 4096;

static void die_perror(const char* msg) {
    std::fprintf(stderr, "%s: %s\n", msg, std::strerror(errno));
    std::exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: %s <file name>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char* file_path = argv[1];

    // 1) Open existing message queue (do NOT create; fail if missing)
    mqd_t mqd = mq_open(QUEUE_NAME, O_WRONLY);
    if (mqd == (mqd_t)-1) {
        die_perror("Failed to open message queue 'cpsc351messagequeue'");
    }


    // 2) Open the file to send
    int fd = ::open(file_path, O_RDONLY);
    if (fd == -1) {
        mq_close(mqd);
        die_perror("Failed to open input file");
    }

    // 3) Read <= 4096 bytes and send with priority 1, until EOF
    char buffer[CHUNK_SIZE];
    while (true) {
        ssize_t n = ::read(fd, buffer, CHUNK_SIZE);
        if (n == -1) {
            // Read error
            int saved = errno;
            ::close(fd);
            mq_close(mqd);
            errno = saved;
            die_perror("File read failed");
        }
        if (n == 0) {
            // EOF: send empty message with priority 2 to signal completion
            if (mq_send(mqd, buffer, 0, /*prio*/2) == -1) {
                int saved = errno;
                ::close(fd);
                mq_close(mqd);
                errno = saved;
                die_perror("mq_send (EOF signal) failed");
            }
            break; // done
        }

        // Send the chunk with priority 1
        if (mq_send(mqd, buffer, static_cast<size_t>(n), /*prio*/1) == -1) {
            int saved = errno;
            ::close(fd);
            mq_close(mqd);
            errno = saved;
            die_perror("mq_send (data) failed");
        }
    }

    ::close(fd);
    mq_close(mqd);
    return EXIT_SUCCESS;
}
