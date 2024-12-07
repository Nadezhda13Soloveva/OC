#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include "unistd.h"
#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node {
    int id;
    pid_t pid;
    int port;
    int parent_id;
    struct Node* next;
} Node;

extern Node* head; // начало списка узлов

Node* find_node(int id);
void add_node(Node* new_node);

#ifdef __cplusplus
}
#endif

#endif 