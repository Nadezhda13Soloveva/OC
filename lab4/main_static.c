#include <stdio.h>
#include <stdlib.h> // Для atoi и atol
#include <ctype.h>
#include "src/lib1.c" // Линкуемся с первой библиотекой

// Чек на положительное число
int is_positive_integer(const char* str) {
    if (*str == '\0') return 0; 
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0; 
        str++;
    }
    return 1; 
}

int main() {
    printf("Режимы:\n1 - Расчет числа 'e',\n2 - Перевод числа X из 10СС в 2CC,\n3 - Выход.\n");
    while (1){
        char input[256];
        printf("Введите команду (режим аргумент): ");
        fgets(input, sizeof(input), stdin);
        int choice;
        char arg[256]; // Буфер для аргумента
        sscanf(input, "%d %s", &choice, arg);

        if (choice == 1) {
            if (!is_positive_integer(arg)) {
                printf("Ошибка: аргумент должен быть положительным целым числом.\n");
            } else {
                int x = atoi(arg);
                printf("Результат E(%d) = %f\n", x, E(x));
            }
        } else if (choice == 2) {
            if (!is_positive_integer(arg)) {
                printf("Ошибка: аргумент должен быть положительным целым числом.\n");
            } else {
                long x = atol(arg);
                char* result = translation(x);
                printf("Результат перевода: %s\n", result);
                free(result);
            }
        } else if (choice == 3) {
            printf("Завершение работы\n");
            break;
        } else {
            printf("Некорреткный ввод :(\nПопробуйте заново\n");
        }
    }
    return 0;
}
