#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 256

double divide_numbers(char *input) {
    char *token = strtok(input, " ");

    // Проверка первого числа
    for (int i = 0; token[i] != '\0'; i++) {
        if (!isdigit(token[i]) && token[i] != '-') {
            fprintf(stderr, "Ошибка: обнаружен посторонний символ.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    double result = atof(token);
    double number;

    while ((token = strtok(NULL, " ")) != NULL) {

        for (int i = 0; token[i] != '\0'; i++) {
            if (!isdigit(token[i]) && token[i] != '-') {
                fprintf(stderr, "Ошибка: обнаружен посторонний символ.\n");
                exit(EXIT_FAILURE);
            }
        }

        number = atof(token);
        if (number == 0) {
            fprintf(stderr, "Ошибка: деление на ноль.\n");
            exit(EXIT_FAILURE);
        }
        
        result /= number;
    }

    return result;
}

int main() {
    char buffer[BUFFER_SIZE];
    char filename[BUFFER_SIZE];

    fgets(filename, BUFFER_SIZE, stdin);
    filename[strcspn(filename, "\n")] = '\0'; 

    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUFFER_SIZE, stdin)) {

        buffer[strcspn(buffer, "\n")] = '\0';

        double result = divide_numbers(buffer);
        fprintf(file, "%.2f\n", result);
    }

    fclose(file);
    return 0;
}
