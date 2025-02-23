#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <stdbool.h>
#include <errno.h>

int main() {
    sem_t* sem;
    FILE* file;
    const char* sem_name = "/sem";

    sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            sem = sem_open(sem_name, 0);
            if (sem == SEM_FAILED) {
                perror("sem_open (existing)");
                return 1;
            }
            printf("Семафор открыт (уже существовал).\n");
        } else {
            perror("sem_open");
            return 1;
        }
    } else {
        printf("Семафор создан и открыт.\n");
    }

    file = fopen("output.txt", "a");
    if (!file) {
        perror("fopen");
        sem_close(sem);
        sem_unlink(sem_name);
        return 1;
    }

    char symbol = '2';

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    while (1) {
        sem_wait(sem);

        for (unsigned short i = 0; i < 10; i++) {
            if (fputc(symbol, file) == EOF) {
                perror("fputc");
                fclose(file);
                sem_close(sem);
                sem_unlink(sem_name);
                printf("Программа завершена из-за ошибки записи в файл.\n");
                return 1;
            }
            fflush(file);
            printf("%c", symbol);
            fflush(stdout);
            sleep(1);
        }

        sem_post(sem);
        sleep(1);

        int ret = ppoll(fds, 1, &(struct timespec){0, 0}, NULL);
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                char buf[10];
                if (fgets(buf, sizeof(buf), stdin) != NULL && buf[0] == '\n') {
                    printf("Нажата клавиша. Завершение программы.\n");
                    break;
                }
            }
        } else if (ret == -1) {
            perror("ppoll");
            break;
        }
    }
    fclose(file);
    sem_close(sem);
    sem_unlink(sem_name);
    return 0;
}
