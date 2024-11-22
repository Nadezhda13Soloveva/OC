#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdlib>

extern "C" {
#include "tools.h"
}

#define SHM_NAME "/shm_channel"

shared_data* shm;
int fd;

// Вспомогательное: Инициализация разделяемой памяти 
void SetUpSharedMemory() {
    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ASSERT_NE(fd, -1) << "Failed to open shared memory";

    ASSERT_EQ(ftruncate(fd, sizeof(shared_data)), 0) << "Failed to set size";

    shm = static_cast<shared_data*>(mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    ASSERT_NE(shm, MAP_FAILED) << "Failed to map memory";

    newSem(shm);
}

// Вспомогательное: Очистка разделяемой памяти
void CleanSharedMemory() {
    destroy(shm);
    munmap(shm, sizeof(shared_data));
    close(fd);
    shm_unlink(SHM_NAME);
}

// Всомогательное: Выполнение команды
bool performDivision(const char* input, double& result) {
    char* rest = strdup(input);
    if (!rest) {
        return false; // Не удалось выделить память
    }
    char* original = rest; // Сохраним оригинальный указатель для последующего освобождения

    char* token;
    double first_num;
    bool is_first = true;
    bool is_error = false;

    while ((token = strtok_r(rest, " ", &rest))) {
        char* endptr;
        double num = strtod(token, &endptr);

        if (*endptr != '\0') {
            is_error = true;
            break;
        }

        if (is_first) {
            first_num = num;
            result = first_num;
            is_first = false;
        } else {
            if (num == 0.0) {
                is_error = true;
                break;
            }
            result /= num;
        }
    }

    free(original); // освобождается память, выделенная для дубликата строки
    return !is_error;
}


// Проверка передачи корректных данных в разделяемую память
TEST(SharedMemoryTests, TestSetData) {
    SetUpSharedMemory();
    const char* input = "42 2";
    setData(shm, const_cast<char*>(input));
    ASSERT_STREQ(shm->data, input);
    CleanSharedMemory();
}

// Проверка передачи некорректных данных в разделяемую память
TEST(SharedMemoryTests, TestInvalidInput) {
    SetUpSharedMemory();
    const char* input = "abc";
    setData(shm, const_cast<char*>(input));
    ASSERT_STREQ(shm->data, input);
    CleanSharedMemory();
}

// Проверка выявления ошибки при делении на 0
TEST(SharedMemoryTests, TestDivisionByZero) {
    double result;
    ASSERT_FALSE(performDivision("42 0", result));
}

// Проверка передачи сигнала выхода в разделяемую память
TEST(SharedMemoryTests, TestExitCommand) {
    SetUpSharedMemory();
    const char* input = "EXIT";
    setData(shm, const_cast<char*>(input));
    ASSERT_STREQ(shm->data, input);
    CleanSharedMemory();
}

// Проверка корректного деления для 2-х чисел
TEST(SharedMemoryTests, TestCorrectDivision) {
    double result;
    ASSERT_TRUE(performDivision("42 2", result));
    ASSERT_DOUBLE_EQ(result, 21.0);
}

// Проверка корректного деления с отрицательными делителями
TEST(SharedMemoryTests, TestMultipleDivisions) {
    double result;
    ASSERT_TRUE(performDivision("100 -2 5", result));
    ASSERT_DOUBLE_EQ(result, -10.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
