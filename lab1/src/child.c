#include "utils.h"

#include <ctype.h>

int main(const int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Necessary arguments were not provided\n");
        exit(EXIT_FAILURE);
    }

    FILE* out = fopen(argv[1], "w");
    if (!out) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }

    char* input = NULL;
    int strLength;

    while ((input = ReadString(stdin)) != NULL) {
        strLength = strlen(input);
        
        int firstNumber = 0;
        int divisor;
        char flag = 0;

        // Преобразование строки в числа
        // Здесь мы ищем первое число
        char* token = strtok(input, " ");
        if (token != NULL) {
            firstNumber = atoi(token);
        }

        // Обработка последующих чисел
        token = strtok(NULL, " ");
        while (token != NULL) {
            divisor = atoi(token);
            
            // Проверка на деление на ноль
            if (divisor == 0) {
                printf("Division by zero detected. Exiting.\n");
                fclose(out);
                free(input);
                exit(EXIT_FAILURE);
            }

            // Выполняем деление
            firstNumber /= divisor;

            // Переход к следующему числу
            token = strtok(NULL, " ");
            flag = 1; // Устанавливаем флаг, что было хотя бы одно деление
        }

        // Если были произведены деления, записываем результат
        if (flag == 1) {
            fprintf(out, "%d\n", firstNumber);
        }

        free(input);
    }

    fclose(out);
    return 0;
}