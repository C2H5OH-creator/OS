#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* pthread_1(void* arg) {
    // Разрешаем отмену работы потока
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // Устанавливаем отложенный режим
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    printf("Поток 1 начал работу отложенным режимом отмены\n");
    int* flag1 = (int*) arg;
    while(*flag1) {
        printf("%d",*flag1);
        fflush(stdout);
        sleep(1);
    }

    printf("Поток 1 закончил работу отложено\n");
    pthread_exit((void*)1);
}

void* pthread_2(void* arg) {
    // Разрешаем отмену работы потока
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // Устанавливаем асинхронный режим
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); 

    printf("Поток 2 начал работу с асинхронным режимом отмены\n");
    int* flag2 = (int*) arg;
    while(*flag2) {
        printf("%d", *flag2);
        fflush(stdout);
        sleep(1);
    }

    printf("Поток 2 закончил работу асинхронно\n");
    pthread_exit((void*)2);

}

int main() {
    int flag1 = 1, flag2 = 2;
    pthread_t thread_id_1, thread_id_2;
    void *exitcode1, *exitcode2;

    printf("Основная программа начала работу\n");

    pthread_create(&thread_id_1, NULL, pthread_1, &flag1);
    pthread_create(&thread_id_2, NULL, pthread_2, &flag2);
    printf("Программа ожидает нажатия клавиши\n");
    getchar();
    
    
    flag1 = 0;
    flag2 = 0;

    pthread_join(thread_id_1, &exitcode1);
    pthread_join(thread_id_2, &exitcode2);
    
    printf("Exitcode первого потока = %d\n",*(int*)&exitcode1);
    printf("Exitcode второго потока = %d\n",*(int*)&exitcode2);
    printf("Основная программа завершила работу\n");
    return 0;

}
