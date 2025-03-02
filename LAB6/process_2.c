#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

const int SHM_SIZE = 1024;

int flag2 = 0;
sem_t *sem_write;
sem_t *sem_read;
int shm_fd;
void *shm_ptr;

void* thread_2(void* arg) {
    char local_data[SHM_SIZE];

    while (!flag2) {
        sem_wait(sem_write);

        memcpy(local_data, shm_ptr, SHM_SIZE);
        printf("Прочитанный IP адрес: %s\n", local_data);

        sem_post(sem_read);
    }
    return NULL;
}

int main() {
    const char* SHM_NAME = "/shared_memory";
    const char* SEM_WRITE_NAME = "/sem_write";
    const char* SEM_READ_NAME = "/sem_read";

    pthread_t thread_id_2;

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open");
            exit(EXIT_FAILURE);
        }
        ftruncate(shm_fd, SHM_SIZE);
     }

    sem_write = sem_open(SEM_WRITE_NAME, 0);
    if (sem_write == SEM_FAILED) {
        sem_write = sem_open(SEM_WRITE_NAME, O_CREAT, 0666, 0);
        if (sem_write == SEM_FAILED) {
            perror("sem_open (sem_write)");
            exit(EXIT_FAILURE);
        }
    }

    sem_read = sem_open(SEM_READ_NAME, 0);
    if (sem_read == SEM_FAILED) {
        sem_read = sem_open(SEM_READ_NAME, O_CREAT, 0666, 0);
        if (sem_read == SEM_FAILED) {
            perror("sem_open (sem_read)");
            exit(EXIT_FAILURE);
        }
    }

    shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    pthread_create(&thread_id_2, NULL, thread_2, NULL);

    printf("Нажмите любую клавишу для завершения...\n");
    getchar();

    flag2 = 1;

    pthread_join(thread_id_2, NULL);

    sem_close(sem_read);
    sem_close(sem_write);

    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);

    return 0;
}
