#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        close(pipefd[1]); 
        
        dup2(pipefd[0], STDIN_FILENO);
        execl("../lab1/child", "child", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else { 
        close(pipefd[0]); 

        printf("Введите название файла: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        write(pipefd[1], buffer, strlen(buffer));

        printf("Введите команду (или пустую строку для завершения):\n");
        while (1) {
            fgets(buffer, BUFFER_SIZE, stdin);

            if (buffer[0] == '\n') { 
                break;
            }

            write(pipefd[1], buffer, strlen(buffer));
        }

        close(pipefd[1]);
        wait(NULL); 
    }
    return 0;
}
