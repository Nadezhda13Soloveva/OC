#ifndef TOOLS_H
#define TOOLS_H

#include <semaphore.h>
#include <stdio.h>

// Структурка для взаимодействия процессов
typedef struct {
    sem_t sem_parent;
    sem_t sem_child;
    char data[256];
} shared_data;

void newSem(shared_data *shm);
void setData(shared_data *shm, char* str);
char* readCommand(FILE* stream);
void destroy(shared_data *shm);

#endif
