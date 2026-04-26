#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static volatile sig_atomic_t stop = 0;
static int count = 0;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s file\n", prog);
    exit(EXIT_FAILURE);
}

static void handler(int sig) {
    (void)sig;
    stop = 1;
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
            default:
                usage(argv[0]);
        }
    }

    if (argc - optind != 1)
        usage(argv[0]);

    // 1. Получить из аргументов командной строки имя файла myfile.
    const char *file = argv[optind];

    char lck[256];
    snprintf(lck, sizeof(lck), "%s.lck", file);

    signal(SIGINT, handler);

    pid_t pid = getpid();

    while (!stop) {
        int fd;

        // 2. Проверить существование файла .lck: если не существует, то создать; если существует, то ожидать его удаления.
        while (1) {
            fd = open(lck, O_CREAT | O_EXCL | O_WRONLY, 0644);
            if (fd >= 0)
                break;
            nanosleep(&(struct timespec){0, 10000000}, NULL);
        }

        // 3. Записать в этот файл свой pid.
        char buf[32];
        int len = snprintf(buf, sizeof(buf), "%d\n", pid);

        if (write(fd, buf, len) != len)
            die("write");

        close(fd);
        
        // 4. Выполнить чтение/запись исходного файла myfile, добавить сон для наглядности.
        sleep(1);

        // 5. Проверить, что файл .lck ещё существует и в нём записан свой pid.
        fd = open(lck, O_RDONLY);
        if (fd < 0)
            die("open lck read");

        char readbuf[32] = {0};
        if (read(fd, readbuf, sizeof(readbuf)) < 0)
            die("read");

        close(fd);

        if (atoi(readbuf) != pid) {
            fprintf(stderr, "lock corrupted\n");
            exit(EXIT_FAILURE);
        }

        if (unlink(lck) < 0)
            die("unlink");

        count++;
        nanosleep(&(struct timespec){0, 10000000}, NULL);
    }

    int statfd = open("stat.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (statfd < 0)
        die("open stat");

    char out[64];
    int l = snprintf(out, sizeof(out), "pid %d: %d\n", pid, count);

    if (write(statfd, out, l) != l)
        die("write stat");

    close(statfd);

    return 0;
}