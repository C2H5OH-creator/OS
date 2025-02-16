#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>


int flag1 = 1, flag2 = 2;
int pipe_fd[2];

void* pthread_1(void* arg) {
    printf("Поток 1 начал работу\n");
    int* flag1 = (int*) arg;

    char buffer[1024];
    struct hostent host_info, *host_result;
    int h_errnop;

    while(*flag1) {
         int res = gethostbyname_r("www.google.ru",
				   &host_info, buffer,
				   sizeof(buffer),
				   &host_result,
				   &h_errnop);

	if (res == 0 && host_result != NULL) {
            char *ip = inet_ntoa(*((struct in_addr*)host_info.h_addr_list[0]));
            snprintf(buffer, sizeof(buffer), "IP: %s", ip);

            if (write(pipe_fd[1], buffer, strlen(buffer) + 1) == -1) {
                perror("Ошибка записи в канал");
            }
        } else {
            printf("Ошибка получения информации о хосте\n");
        }
	sleep(1);
    }

    printf("Поток 1 закончил работу\n");
    pthread_exit((void*)1);
}

void* pthread_2(void* arg) {
    printf("Поток 2 начал работу\n");
    int* flag2 = (int*) arg;
    char buffer[1024];

    while(*flag2) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
        if (bytes_read > 0) {
            printf("Получено сообщение: %s\n", buffer);
        } else if (bytes_read == -1) {
            perror("Ошибка чтения из канала");
        }
    }

    printf("Поток 2 закончил работу\n");
    pthread_exit((void*)2);

}

int main() {
    pthread_t thread_id_1, thread_id_2;
    void *exitcode1, *exitcode2;

    if (pipe(pipe_fd) == -1) {
        perror("Ошибка создания канала");
        exit(EXIT_FAILURE);
    }

    printf("Основная программа начала работу\n");

    pthread_create(&thread_id_1, NULL, pthread_1, &flag1);
    pthread_create(&thread_id_2, NULL, pthread_2, &flag2);
    printf("Программа ожидает нажатия клавиши\n");
    getchar();

    flag1 = 0;
    flag2 = 0;

    close(pipe_fd[1]);
    pthread_join(thread_id_1, &exitcode1);
    pthread_join(thread_id_2, &exitcode2);

    close(pipe_fd[0]);


    printf("Основная программа завершила работу\n");
    return 0;

}
