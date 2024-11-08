#include "lab2.h"
#include <stdio.h>
#include <stdlib.h>

void* convolveCells(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int** inputMatrix = data->inputMatrix;
    int** outputMatrix = data->outputMatrix;
    int M = data->M;
    int outputSize = data->outputSize;

    for (int idx = data->startCell; idx < data->endCell; idx++) {
        int row = idx / outputSize;
        int col = idx % outputSize;
        outputMatrix[row][col] = 0;

        for (int ki = 0; ki < M; ki++) {
            for (int kj = 0; kj < M; kj++) {
                outputMatrix[row][col] += inputMatrix[row + ki][col + kj];
            }
        }
    }
    return NULL;
}

void printMatrix(int** matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%4d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int** createMatrix(int N) {
    int** matrix = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        matrix[i] = (int*)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * N + j + 1;
        }
    }
    return matrix;
}

ThreadPool* createThreadPool(int numThreads) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    pool->threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));
    pool->threadData = (ThreadData*)malloc(numThreads * sizeof(ThreadData));
    pool->numThreads = numThreads;
    return pool;
}

void destroyThreadPool(ThreadPool* pool) {
    free(pool->threads);
    free(pool->threadData);
    free(pool);
}

void runConvolutionTasks(ThreadPool* pool, int** matrix, int** output, int N, int M, int numCells) {
    int outputSize = N - M + 1;
    int usedThreads = (numCells < pool->numThreads) ? numCells : pool->numThreads;

    int cellsPerThread = numCells / usedThreads;
    int extraCells = numCells % usedThreads;

    int startCell = 0;
    for (int i = 0; i < usedThreads; i++) {
        int endCell = startCell + cellsPerThread + (i == usedThreads - 1 ? extraCells : 0);

        pool->threadData[i].inputMatrix = matrix;
        pool->threadData[i].outputMatrix = output;
        pool->threadData[i].N = N;
        pool->threadData[i].M = M;
        pool->threadData[i].outputSize = outputSize;
        pool->threadData[i].startCell = startCell;
        pool->threadData[i].endCell = endCell;

        pthread_create(&pool->threads[i], NULL, convolveCells, (void*)&pool->threadData[i]);
        startCell = endCell;
    }

    for (int i = 0; i < usedThreads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

int** convolve(ThreadPool* pool, int** matrix, int N, int M) {
    int outputSize = N - M + 1;
    int** output = (int**)malloc(outputSize * sizeof(int*));
    for (int i = 0; i < outputSize; i++) {
        output[i] = (int*)malloc(outputSize * sizeof(int));
    }

    int numCells = outputSize * outputSize;
    runConvolutionTasks(pool, matrix, output, N, M, numCells);

    return output;
}

void freeMatrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}
