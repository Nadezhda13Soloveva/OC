#include "../include/node_manager.h"
#include <stdlib.h>
#include "unistd.h"
#include "sys/types.h"

Node* head = NULL; // изначально список пуст

// Добавляем новый узел в начало списка
void add_node(Node* new_node) {
    new_node->next = head;
    head = new_node;
}


// Проходим по списку начиная с head и ищем узел по id
Node* find_node(int id) {
    Node* current = head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
} 