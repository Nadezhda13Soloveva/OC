#ifndef COMMON_H
#define COMMON_H

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_MSG_LEN 256
#define BASE_PORT 5550 // для вычисления портов для узлов

typedef struct Node {
    int id;
    pid_t pid;
    int port;
    int parent_id;
    struct Node* next;
} Node;

// Коды команд
#define CMD_CREATE 1
#define CMD_EXEC 2
#define CMD_PING 3

// Коды ответов
#define REPLY_OK 0
#define REPLY_ERROR 1

// Для словаря
#define MAX_DICT_ENTRIES 100
#define MAX_KEY_LEN 64

// Структурка для записей словаря
typedef struct {
    char key[MAX_KEY_LEN];
    int value;
    int is_set; // флаг установлено ли значение
} DictEntry;

#endif