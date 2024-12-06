#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ctype.h>
#include "tools.h"

int main(int argc, char* argv[]) {
    // чек параметров
    if (argc != 3) {
        printf("Necessary arguments were not provided\n");
        exit(EXIT_FAILURE);
    }

    // открываю объект разделяемой памяти в режиме чтения и записи
    int shm_fd = shm_open(argv[1], O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared_memory\n");
        exit(EXIT_FAILURE);
    }
    // отображаю объект разделяемой памяти в адресное пространство в режиме чтения и записи, изменения будут видны другим процессам, отображение с начала объекта (смещение 0)
    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("Failed to map to the address space (mmap)\n");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    FILE *output_file = fopen(argv[2], "w");
    if (!output_file) {
        perror("Failed to open output file\n");
        munmap(shm, sizeof(shared_data));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    int is_error = 0; 

    while (1) {
        sem_wait(&shm->sem_child); // ожидаем сигнал для обработки команд

        if (strcmp(shm->data, "EXIT") == 0) {
            break;
        }

        char* token;
        char* rest = shm->data; // оставшаяся строка
        double first_num;
        double result;
        int is_first = 1;
        is_error = 0; // сбрасываем флаг ошибки перед обработкой новой команды

        while ((token = strtok_r(rest, " ", &rest))) {
            char* endptr; // указатель начала несоответствия для перевода в число
            double num = strtod(token, &endptr);

            if (*endptr != '\0') {
                fprintf(stderr, "Error: invalid input\n");
                is_error = 1;
                break;
            }

            if (is_first) {
                first_num = num;
                result = first_num;
                is_first = 0;
            } else {
                if (num == 0.0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    is_error = 1;
                    break;
                }
                result /= num;
            }
        }

        // передача сигнала об ошибке родителю
        if (is_error) {
            setData(shm, "ERROR");
            
        } else {
            fprintf(output_file, "Result: %.2f\n", result);
            fflush(output_file);
        }

        sem_post(&shm->sem_parent);
    }

    fclose(output_file);

    destroy(shm);

    munmap(shm, sizeof(shared_data));
    close(shm_fd);

    return is_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
