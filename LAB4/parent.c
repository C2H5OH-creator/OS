#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    printf("Родительский процесс: PID = %d, PPID = %d\n", getpid(), getppid());
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        char* childs_name = "./child";
        char *args[] = {"Пример аргумента №1 переданного в дочерний процесс",
                        "Пример аргумента №2 переданного в дочерний процесс",
                        "Пример аргумента №3 переданного в дочерний процесс",
                        "Пример аргумента №4 переданного в дочерний процесс",
                        NULL};
        execvp(childs_name, args);
        perror("execvp");
        exit(1);
    }
    else {
        int status = 0;
        //const char frames[] = {'\\', '|', '/', '-'};
        //unsigned short frames_index = 0;
        while (waitpid(pid, &status, WNOHANG) == 0) {
            //printf("\rЖдём... %c", frames[frames_index]);
            printf("\rЖдём... ");
            fflush(stdout);
            //frames_index = (frames_index++) % 4;
            usleep(200000);
        }
        if (WIFEXITED(status)) {
            printf("\nДочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        }
        else {
            printf("\nДочерний процесс завершился некорректно\n");
        }
    }
    return 0;
}
