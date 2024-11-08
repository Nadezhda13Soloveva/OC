#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <utility>
#include <fstream>

extern "C" {
    #include "lab2.h" 
}

// Вспомогательная функция для выполнения программы и получения ошибки
int executeProgram(const char* command) {
    int result = system(command);
    return result;
}


// Тест: Проверка параметров
TEST(ConvolutionTest, ParameterCheck) {

    int result = executeProgram("../lab2/lab2_exe 5 0 1 2");
    ASSERT_NE(result, 0);

    result = executeProgram("../lab2/lab2_exe 0 3 1 2");
    ASSERT_NE(result, 0);

    result = executeProgram("../lab2/lab2_exe 5 6 1 2");
    ASSERT_NE(result, 0);

    result = executeProgram("../lab2/lab2_exe 5 5 2 1");
    ASSERT_NE(result, 0);

    result = executeProgram("../lab2/lab2_exe 5 3 0 2");
    ASSERT_NE(result, 0);

    result = executeProgram("../lab2/lab2_exe 5 3 1 0");
    ASSERT_NE(result, 0);
}


// Тест: Проверка корректности свертки (базово 1 поток)
TEST(ConvolutionTest, Correctness) {
    int N = 5;
    int M = 3;
    int maxThreads = 1;  
    int** matrix = createMatrix(N);
    
    ThreadPool* pool = createThreadPool(maxThreads);

    int** result = convolve(pool, matrix, N, M);

    int expected[3][3] = {
        {63, 72, 81},
        {108, 117, 126},
        {153, 162, 171}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ASSERT_EQ(result[i][j], expected[i][j]);
        }
    }

    freeMatrix(result, 3);
    freeMatrix(matrix, N);
    destroyThreadPool(pool);
}


// Тест: Проверка свертки с многопоточностью
TEST(ConvolutionTest, MultiThreading) {
    int N = 5;
    int M = 3;
    int maxThreads = 3;  
    int** matrix = createMatrix(N);
    
    ThreadPool* pool = createThreadPool(maxThreads);

    int** result = convolve(pool, matrix, N, M);

    int expected[3][3] = {
        {63, 72, 81},
        {108, 117, 126},
        {153, 162, 171}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ASSERT_EQ(result[i][j], expected[i][j]);
        }
    }

    freeMatrix(result, 3);
    freeMatrix(matrix, N);
    destroyThreadPool(pool);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
