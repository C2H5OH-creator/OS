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

void* send_requests(void* arg) {
    int count = 1;
    char buffer[1024];
    struct hostent ret;
    struct hostent *result;
    int h_errnop;
    char *temp_buffer = malloc(8192); // Буфер для gethostbyname_r

    while (!send_flag) {
        if (gethostbyname_r("www.google.ru", &ret, temp_buffer, 8192, &result, &h_errnop) != 0) {
            perror("gethostbyname_r");
            continue;
        }

        char *ip = inet_ntoa(*((struct in_addr *)ret.h_addr_list[0]));
        snprintf(buffer, sizeof(buffer), "%d:%s", count, ip);
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Отправлен запрос №%d с IP Google: %s\n", count++, ip);
        sleep(1);
    }
    
    free(temp_buffer);
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

    while (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        sleep(1);
    }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getsockname(client_socket, (struct sockaddr*)&client_addr, &len);
    printf("Адрес клиента: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    pthread_t send_thread, recv_thread;
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