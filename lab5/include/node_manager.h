#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include "../include/common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Node* head; // начало списка узлов

Node* find_node(int id);
void add_node(Node* new_node);

#ifdef __cplusplus
}
#endif

#endif 