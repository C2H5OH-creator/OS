#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Временно устанавливаем значение текущей директории, как переменную PATH
    char *old_path = getenv("PATH");
    char new_path[1024];
    snprintf(new_path, sizeof(new_path), ".:%s", old_path);
    setenv("PATH", new_path, 1);

    printf("Родительский процесс: PID = %d, PPID = %d\n", getpid(), getppid());
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        char* childs_name = "child";
        execvp(childs_name, argv);
        perror("execvp");
        exit(1);
    }
    else {
        int status = 0;
        while (waitpid(pid, &status, WNOHANG) == 0) {
            printf("\rЖдём... ");
            fflush(stdout);
            usleep(200000);
        }
        if (WIFEXITED(status)) {
            printf("\nДочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        }
        else {
            printf("\nДочерний процесс завершился некорректно\n");
        }
    }
    // Возвращаем старую переменную PATH как было
    setenv("PATH", old_path, 1);
    return 0;
}
