#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/queue.h>
#include <signal.h>
#include <sys/utsname.h>
#include <netdb.h>

struct request {
    int number;
    char ip[16];
    STAILQ_ENTRY(request) entries;
};

STAILQ_HEAD(stailhead, request);

int server_socket, client_socket = -1;
struct stailhead request_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int accept_flag = 0;
volatile int receive_flag = 0;
volatile int process_flag = 0;

void* receive_requests(void* arg) {
    char buffer[1024];
    while (!receive_flag) {
        if (client_socket == -1) {
            sleep(1);
            continue;
        }
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            
            int num;
            char ip[16];
            sscanf(buffer, "%d:%s", &num, ip);
            
            struct request* req = malloc(sizeof(struct request));
            req->number = num;
            strncpy(req->ip, ip, sizeof(req->ip) - 1);
            req->ip[sizeof(req->ip) - 1] = '\0';
            
            pthread_mutex_lock(&queue_mutex);
            STAILQ_INSERT_TAIL(&request_queue, req, entries);
            pthread_mutex_unlock(&queue_mutex);
            printf("Получен запрос №%d\n", num);
        } else if (bytes == 0) {
            printf("Клиент отключился\n");
            break;
        } else {
            perror("recv");
        }
        sleep(1);
    }
    receive_flag = 1;
    return NULL;
}

void* process_requests(void* arg) {
    while (!process_flag) {
        pthread_mutex_lock(&queue_mutex);
        if (!STAILQ_EMPTY(&request_queue)) {
            struct request* req = STAILQ_FIRST(&request_queue);
            STAILQ_REMOVE_HEAD(&request_queue, entries);
            pthread_mutex_unlock(&queue_mutex);

            char response[1024];
            struct hostent *result = gethostbyname("www.google.ru");
            
            if (result == NULL) {
                snprintf(response, sizeof(response), 
                        "Ответ №%d: Ошибка получения IP адреса Google", 
                        req->number);
                herror("gethostbyname");
            } else {
                char *ip = inet_ntoa(*((struct in_addr *)result->h_addr_list[0]));
                snprintf(response, sizeof(response), 
                        "Ответ №%d: IP адрес Google: %s", 
                        req->number, ip);
            }

            ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);
            if (bytes_sent == -1) {
                perror("send");
            } else {
                printf("Отправлен ответ №%d\n", req->number);
            }

            free(req);
        } else {
            pthread_mutex_unlock(&queue_mutex);
            sleep(1);
        }
    }
    process_flag = 1;
    return NULL;
}

void* accept_connections(void* arg) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    while (!accept_flag) {
        int new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len);
        if (new_socket >= 0) {
            client_socket = new_socket;
            printf("Клиент подключился: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            pthread_t recv_thread, proc_thread;
            pthread_create(&recv_thread, NULL, receive_requests, NULL);
            pthread_create(&proc_thread, NULL, process_requests, NULL);
            break;
        }
        sleep(1);
    }
    accept_flag = 1;
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(7000),
        .sin_addr.s_addr = INADDR_ANY
    };

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_socket, F_SETFL, O_NONBLOCK);
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    bind(server_socket, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_socket, 5);
    STAILQ_INIT(&request_queue);

    pthread_t accept_thread;
    pthread_create(&accept_thread, NULL, accept_connections, NULL); 

    printf("Сервер запущен. Нажмите Enter чтобы выйти...\n");
    getchar();

    accept_flag = 1;
    receive_flag = 1;
    process_flag = 1;

    pthread_join(accept_thread, NULL);
    if (client_socket != -1) {
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
    }
    close(server_socket);
    return 0;
}