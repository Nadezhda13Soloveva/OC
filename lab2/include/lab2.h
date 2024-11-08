#ifndef LAB2_H
#define LAB2_H

#include <pthread.h>

// Структурка для рабочей задачи для потока
typedef struct {
    int** inputMatrix;
    int** outputMatrix;
    int N;
    int M;
    int startCell;
    int endCell;
    int outputSize;
} ThreadData;

// Пул потоков
typedef struct {
    pthread_t* threads;
    ThreadData* threadData;
    int numThreads;
} ThreadPool;


void* convolveCells(void* arg);
void printMatrix(int** matrix, int rows, int cols);
int** createMatrix(int N);
ThreadPool* createThreadPool(int numThreads);
void destroyThreadPool(ThreadPool* pool);
void runConvolutionTasks(ThreadPool* pool, int** matrix, int** output, int N, int M, int numCells);
int** convolve(ThreadPool* pool, int** matrix, int N, int M);
void freeMatrix(int** matrix, int rows);


#endif // LAB2_H
