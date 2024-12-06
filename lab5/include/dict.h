#ifndef DICT_H
#define DICT_H

#include <string.h>

#define MAX_KEY_LEN 64
#define MAX_DICT_SIZE 100

// Тоже структурка для записей словаря
typedef struct {
    char key[MAX_KEY_LEN];
    int value;
} DictEntry;

// Словарь: массив записей и размер
typedef struct {
    DictEntry entries[MAX_DICT_SIZE];
    int size;
} Dictionary;

void dict_init(Dictionary* dict);
int dict_set(Dictionary* dict, const char* key, int value);
int dict_get(Dictionary* dict, const char* key, int* value);

#endif