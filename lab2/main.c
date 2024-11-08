#include "lab2.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <N> <M> <K> <MAX_THREADS>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int K = atoi(argv[3]);
    int maxThreads = atoi(argv[4]);
    
    if (M <= 0 || N <= 0) {
        fprintf(stderr, "Error: Matrix size must be greater than 0.\n");
        return 1;
    }

    if (M > N) {
        fprintf(stderr, "Error: Convolution window size must be less than or equal to the matrix size.\n");
        return 1;
    }
    int finalOutputSize = N - K * (M - 1);
    if (finalOutputSize < 1) {
        fprintf(stderr, "Error: Resulting matrix size after all convolutions must be at least 1x1.\n");
        return 1;
    }

    if (K <= 0) {
        fprintf(stderr, "Error: Convolution filter count must be greater than 0.\n");
        return 1;
    }

    if (maxThreads <= 0) {
        fprintf(stderr, "Error: Number of threads must be greater than 0.\n");
        return 1;
    }

    printf("Matrix size: %d\n", N);
    printf("Convolution window size: %d\n", M);
    printf("Convolution filter count: %d\n", K);
    printf("MAX threads: %d\n", maxThreads);

    clock_t start = clock();

    int** matrix = createMatrix(N);
    if (N < 25) {
        printf("\nOriginal matrix:\n");
        printMatrix(matrix, N, N);
    }

    ThreadPool* pool = createThreadPool(maxThreads);

    int** currentMatrix = matrix;
    for (int i = 0; i < K; i++) {
        currentMatrix = convolve(pool, currentMatrix, N, M);
        int outputSize = N - M + 1;
        if (i + 1 == K && outputSize * outputSize < 576) {
            printf("\nMatrix after convolution:\n");
            printMatrix(currentMatrix, outputSize, outputSize);
        }
        N = outputSize;
    }

    destroyThreadPool(pool);

    freeMatrix(matrix, N);
    if (currentMatrix != NULL) {
        freeMatrix(currentMatrix, N);
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\nUsed threads: %d\n", (N * N < maxThreads) ? (N * N) : maxThreads);
    printf("Time spent: %.5f seconds\n", time_spent);

    return 0;
}
