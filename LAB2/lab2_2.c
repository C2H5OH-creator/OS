#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int flag1 = 1, flag2 = 2;
pthread_mutex_t mutex;

void* pthread_1(void* arg) {
    printf("Поток 1 начал работу\n");
    int* flag1 = (int*) arg;
    while(*flag1) {
	pthread_mutex_lock(&mutex);
        for(int i = 0; i < 10; i++){
	    printf("1");
            fflush(stdout);
            sleep(1);
	}
	sleep(1);
	pthread_mutex_unlock(&mutex);
    }

    printf("Поток 1 закончил работу\n");
    pthread_exit((void*)1);
}

void* pthread_2(void* arg) {
    printf("Поток 2 начал работу\n");
    int* flag2 = (int*) arg;
    while(*flag2) {
	pthread_mutex_lock(&mutex);
        for(int i = 0; i < 10; i++){
	    printf("2");
            fflush(stdout);
            sleep(1);
	}
	pthread_mutex_unlock(&mutex);
	sleep(1);
    }

    printf("Поток 2 закончил работу\n");
    pthread_exit((void*)2);

}

int main() {
    pthread_t thread_id_1, thread_id_2;
    void *exitcode1, *exitcode2;
    pthread_mutex_init(&mutex, NULL);

    printf("Основная программа начала работу\n");

    pthread_create(&thread_id_1, NULL, pthread_1, &flag1);
    pthread_create(&thread_id_2, NULL, pthread_2, &flag2);
    printf("Программа ожидает нажатия клавиши\n");
    getchar();

    flag1 = 0;
    flag2 = 0;

    pthread_join(thread_id_1, &exitcode1);
    pthread_join(thread_id_2, &exitcode2);

    pthread_mutex_destroy(&mutex);

    printf("Основная программа завершила работу\n");
    return 0;

}
