#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "tools.h"

#define SHM_NAME "/shm_channel"

int main(void) {
    printf("Enter the output file name: ");
    char file_name[256];
    scanf("%s", file_name);
    getchar(); // чтобы считать и удалить \n из буфера

    int file = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0) {
        perror("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Failed to open shared_memory\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(shared_data)) == -1) {
        perror("Failed to set size ftruncate\n");
        exit(EXIT_FAILURE);
    }

    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("Failed to map to the address space (mmap)\n");
        exit(EXIT_FAILURE);
    }

    newSem(shm);

    // создаю дочерний и подменяю образ процесса
    pid_t child = fork();
    if (child < 0) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    } else if (child == 0) {
        close(file);
        execl("./child3", "child3", SHM_NAME, file_name, NULL);
        perror("Failed to exec\n");
        exit(EXIT_FAILURE);
    }


    char* input;
    while ((input = readCommand(stdin)) != NULL) {
        if (strlen(input) == 0) {
            free(input);
            break;
        }
        setData(shm, input); // передаю команду в data структурки
        sem_post(&shm->sem_child); // уведомляю дочерний о новой команде
        sem_wait(&shm->sem_parent); // родитель ждёт дочернего

        // реакция на ошибку деления на 0 или некорректного символа
        if (strcmp(shm->data, "ERROR") == 0) {
            fprintf(stderr, "End of work. You can try again)\n");
            free(input);
            break;
        }

        free(input);
    }

    setData(shm, "EXIT"); // уведомляем дочерний о конце
    sem_post(&shm->sem_child);

    waitpid(child, NULL, 0);

    destroy(shm);

    munmap(shm, sizeof(shared_data)); 
    close(fd);
    shm_unlink(SHM_NAME); 

    close(file);

    return 0;
}
