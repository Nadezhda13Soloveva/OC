#include "../include/dict.h"
#include <string.h>

// инит словаря
void dict_init(Dictionary* dict) {
    dict->size = 0;
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        dict->entries[i].is_set = 0; 
    }
}

// добавление значения
int dict_set(Dictionary* dict, const char* key, int value) {
    // обновление существующей
    for (int i = 0; i < dict->size; i++) {
        if (dict->entries[i].is_set && strcmp(dict->entries[i].key, key) == 0) {
            dict->entries[i].value = value;
            return 0;
        }
    }
    
    // добавление новой пары
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        if (!dict->entries[i].is_set) {
            strncpy(dict->entries[i].key, key, MAX_KEY_LEN - 1);
            dict->entries[i].key[MAX_KEY_LEN - 1] = '\0'; 
            dict->entries[i].value = value;
            dict->entries[i].is_set = 1; 
            if (i >= dict->size) {
                dict->size++;
            }
            return 0;
        }
    }
    
    return -1; // словарь переполнен
}

// получение значения
int dict_get(Dictionary* dict, const char* key, int* value) {
    for (int i = 0; i < dict->size; i++) {
        if (dict->entries[i].is_set && strcmp(dict->entries[i].key, key) == 0) {
            *value = dict->entries[i].value;
            return 0;
        }
    }
    return -1; 
}