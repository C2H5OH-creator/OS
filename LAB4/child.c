#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[], char *envp[]){
   for (int i = 0; i < argc; i++) {
    	printf("Аргумент %d: %s\n", i, argv[i]);
    	sleep(1);
    }

   for (int i = 0; i < 10; i++) {
        printf("Дочерный процесс: PID = %d, PPID = %d\n", getpid(), getppid());
        sleep(1);
    }
    exit(5);
}
