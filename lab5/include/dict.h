#ifndef DICT_H
#define DICT_H

// Для словаря
#define MAX_DICT_ENTRIES 100
#define MAX_KEY_LEN 64

// Структурка для записей словаря
typedef struct {
    char key[MAX_KEY_LEN];
    int value;
    int is_set; // флаг установлено ли значение
} DictEntry;

// Словарь: массив записей и размер
typedef struct {
    DictEntry entries[MAX_DICT_ENTRIES];
    int size;
} Dictionary;

void dict_init(Dictionary* dict);
int dict_set(Dictionary* dict, const char* key, int value);
int dict_get(Dictionary* dict, const char* key, int* value);

#endif