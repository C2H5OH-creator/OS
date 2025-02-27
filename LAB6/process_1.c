#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>

int flag1 = 0;
sem_t *sem_write;
sem_t *sem_read;
int shm_fd;
void *shm_ptr;

void* thread_1(void* arg) {
    struct hostent ret, *result;
    char buffer[1024];
    int h_errnop;

    while (!flag1) {
        if (gethostbyname_r("www.google.ru", &ret, buffer, sizeof(buffer), &result, &h_errnop) != 0) {
            perror("gethostbyname_r");
            exit(EXIT_FAILURE);
        }

        char *ip = inet_ntoa(*((struct in_addr *)ret.h_addr_list[0]));
        printf("Записанный IP адрес: %s\n", ip);

        memcpy(shm_ptr, ip, strlen(ip) + 1);

        sem_post(sem_write);

        sem_wait(sem_read);

        sleep(1);
    }
    return NULL;
}

int main() {
    // Константы для удобства
    const char* SHM_NAME = "/shared_memory";
    const char* SEM_WRITE_NAME = "/sem_write";
    const char* SEM_READ_NAME = "/sem_read";
    const int SHM_SIZE = 1024;

    pthread_t thread_id_1;

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    shm_ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_write = sem_open(SEM_WRITE_NAME, O_CREAT, 0666, 0);
    sem_read = sem_open(SEM_READ_NAME, O_CREAT, 0666, 0);

    pthread_create(&thread_id_1, NULL, thread_1, NULL);

    printf("Нажмите любую клавишу для завершения...\n");
    getchar();

    flag1 = 1;

    pthread_join(thread_id_1, NULL);

    sem_close(sem_read);
    sem_unlink(SEM_READ_NAME);
    sem_close(sem_write);
    sem_unlink(SEM_WRITE_NAME);

    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME);

    return 0;
}
