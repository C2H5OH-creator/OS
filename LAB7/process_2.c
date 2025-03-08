#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>

int thread_stop = 0;
int msgid;

typedef struct {
    long mtype;
    char mtext[256];
} TMessage;

void create_file_if_not_exists(const char* filename) {
    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

void* thread_reader(void* arg) {
    TMessage message;
    ssize_t result;

    while (!thread_stop) {
        memset(message.mtext, 0, sizeof(message.mtext));

        result = msgrcv(msgid, &message, sizeof(message.mtext), 1, IPC_NOWAIT);
        if (result > 0) {
            printf("Полученный IP адрес: %s\n", message.mtext);
        } else if (result == -1) {
            perror("msgrcv");
        }

        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    key_t key;
    int mode = 0666;

	const char* filename = "lab7";
	create_file_if_not_exists(filename);

    key = ftok(filename, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    msgid = msgget(key, 0);
    if (msgid == -1) {
        msgid = msgget(key, IPC_CREAT | mode);
        if (msgid == -1) {
            perror("msgget");
            exit(1);
        }
    }

    if (pthread_create(&thread, NULL, thread_reader, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    printf("Нажмите любую клавишу для завершения...\n");
    getchar();

    thread_stop = 1;

    pthread_join(thread, NULL);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
    }

    printf("Программа завершена.\n");
    return 0;
}
