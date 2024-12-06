#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

int is_positive_integer(const char* str) {
    if (*str == '\0') return 0; 
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0; 
        str++;
    }
    return 1;
}

int main() {
    void* handle = NULL; // дескриптор дин библиотеки
    float (*E)(int) = NULL; 
    char* (*translation)(long) = NULL;

    handle = dlopen("./liblib1.so", RTLD_LAZY); // подгружаем lib1
    if (!handle) {
        fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
        return 1;
    }

    // uintptr_t для приведения void* напрямую к указателю на функции
    E = (float (*)(int))(uintptr_t)dlsym(handle, "E");
    translation = (char* (*)(long))(uintptr_t)dlsym(handle, "translation");

    char input[256];
    printf("Режимы:\n0 - Смена реализаций,\n1 - Расчет числа 'e',\n2 - Перевод числа X из 10СС в 2CC или 3CC,\n3 - Выход.\n");
    while (1) {
        printf("Введите команду (режим аргумент): ");
        fgets(input, sizeof(input), stdin);
        int choice;
        char arg[256]; 
        sscanf(input, "%d %s", &choice, arg);

        if (choice == 0) {
            // закрытие дескриптора и освобождение ресурсов
            // переоткрытие дескриптора
            dlclose(handle);
            if (handle == dlopen("./liblib1.so", RTLD_LAZY)) {
                handle = dlopen("./liblib2.so", RTLD_LAZY);
            } else {
                handle = dlopen("./liblib1.so", RTLD_LAZY);
            }
            E = (float (*)(int))(uintptr_t)dlsym(handle, "E");
            translation = (char* (*)(long))(uintptr_t)dlsym(handle, "translation");
            printf("Библиотека переключена.\n");
        } else if (choice == 1) {
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

    dlclose(handle);
    return 0;
}
