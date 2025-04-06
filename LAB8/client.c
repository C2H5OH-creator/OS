#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>

int client_socket;
volatile int send_flag = 0;
volatile int receive_flag = 0;

void* connect_to_server(void* arg) {
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(7000),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };

    while (!send_flag) {
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != -1) {
            printf("Подключено к серверу\n");
            return NULL;
        }
        perror("connect");
        sleep(1);
    }
    return NULL;
}

void* send_requests(void* arg) {
    int count = 1;
    char buffer[1024];
    
    while (!send_flag) {
        snprintf(buffer, sizeof(buffer), "%d", count);
        ssize_t bytes_sent = send(client_socket, buffer, strlen(buffer), 0);
        if (bytes_sent == -1) {
            perror("send");
            sleep(1);
            continue;
        }
        printf("Отправлен запрос №%d\n", count++);
        sleep(1);
    }
    return NULL;
}

void* receive_responses(void* arg) {
    char buffer[1024];
    while (!receive_flag) {
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("Получено: %s\n", buffer);
        } else if (bytes == 0) {
            printf("Сервер отключился\n");
            break;
        } else {
            perror("recv");
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(7000),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };

    pthread_t connect_thread, send_thread, recv_thread;
    pthread_create(&connect_thread, NULL, connect_to_server, NULL);
    pthread_join(connect_thread, NULL);
    pthread_create(&send_thread, NULL, send_requests, NULL);
    pthread_create(&recv_thread, NULL, receive_responses, NULL);

    printf("Клиент запущен. Нажмите Enter чтобы выйти...\n");
    getchar();

    send_flag = 1;
    receive_flag = 1;
    
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    return 0;
}