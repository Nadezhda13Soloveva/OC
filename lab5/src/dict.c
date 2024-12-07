#include "../include/dict.h"
#include <string.h>

// Инит словаря
void dict_init(Dictionary* dict) {
    dict->size = 0;
}

// Добавление пары в словарь (или обновление)
int dict_set(Dictionary* dict, const char* key, int value) {
    // Поиск существующего ключа
    for (int i = 0; i < dict->size; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            dict->entries[i].value = value;
            return 0;
        }
    }
    
    // Добавление нового ключа
    if (dict->size < MAX_DICT_ENTRIES) {
        strncpy(dict->entries[dict->size].key, key, MAX_KEY_LEN - 1);
        dict->entries[dict->size].value = value;
        dict->size++;
        return 0;
    }
    
    return -1;
}


// Получение значения по ключу
int dict_get(Dictionary* dict, const char* key, int* value) {
    for (int i = 0; i < dict->size; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            *value = dict->entries[i].value;
            return 0;
        }
    }
    return -1;
}