#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

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

char* get_websites_ip(const char* address) {
    struct hostent ret, *result;
    char buffer[1024];
    int h_errnop;

    if (gethostbyname_r(address, &ret, buffer, sizeof(buffer), &result, &h_errnop) != 0) {
        perror("gethostbyname_r");
        exit(EXIT_FAILURE);
    }

    if (ret.h_addr_list[0] == NULL) {
        fprintf(stderr, "Не удалось получить IP-адрес для %s\n", address);
        exit(EXIT_FAILURE);
    }

    return strdup(inet_ntoa(*((struct in_addr *)ret.h_addr_list[0])));
}

void* thread_writer(void* arg) {
    char* ip = get_websites_ip("www.github.com");
    TMessage message;
    int result;

    while (!thread_stop) {
        message.mtype = 1;
        snprintf(message.mtext, sizeof(message.mtext), "%s", ip);

        result = msgsnd(msgid, &message, strlen(message.mtext) + 1, IPC_NOWAIT);
        if (result == -1) {
            perror("msgsnd");
        } else {
            printf("Записанный IP адрес: %s\n", message.mtext);
        }

        sleep(1);
    }

    free(ip);
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

    if (pthread_create(&thread, NULL, thread_writer, NULL) != 0) {
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
