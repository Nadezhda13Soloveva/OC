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

    if (pid == 0) { // child
        close(pipefd[1]); // close pipe for child writing
        
        dup2(pipefd[0], STDIN_FILENO);
        execl("../lab1/child", "child", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } 
    else { // parent
        close(pipefd[0]); // close pipe for parent reading

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

        close(pipefd[1]); // close pipe for parent writing
        wait(NULL); 
    }
    return 0;
}
