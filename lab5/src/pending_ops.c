#include "../include/pending_ops.h"
#include "../include/message.h"
#include <string.h>
#include <zmq.h>
#include <stdio.h>
#include <time.h>

#define MAX_PENDING 100
#define DEFAULT_TIMEOUT 1000  // 1 сек по умолчанию

// пустой список операций
PendingOperation pending_ops[MAX_PENDING];
int pending_count = 0;

// Добавляем новую операцию
void add_pending_operation(int id, void* socket, void* context, OperationType type, Message* msg, int timeout_ms) {
    if (pending_count < MAX_PENDING) {
        pending_ops[pending_count].id = id;
        pending_ops[pending_count].socket = socket;
        pending_ops[pending_count].context = context;
        pending_ops[pending_count].type = type;
        memcpy(&pending_ops[pending_count].msg, msg, sizeof(Message));
        pending_ops[pending_count].timestamp = time(NULL);
        pending_ops[pending_count].timeout_ms = timeout_ms;
        pending_count++;
    }
}


// Ответы на ping
void handle_ping_response(int success) {
    if (success) {
        printf("Ok: 1\n");
    } else {
        printf("Ok: 0\n");
    }
    fflush(stdout);
}


// Ответ на exec
void handle_exec_response(PendingOperation* op, int success) {
    if (success) {
        printf("Ok:%d: %s\n", op->id, op->msg.data);
    } else {
        printf("Error:%d: Node is unavailable\n", op->id);
    }
    fflush(stdout);
}


// Ответ на create
void handle_create_response(PendingOperation* op, int success) {
    if (success) {
        printf("Ok: %s\n", op->msg.data);
    } else {
        printf("Error: Parent is unavailable\n");
    }
    fflush(stdout);
}


// Удаляем операцию по индексу
void remove_operation(int index) {
    zmq_close(pending_ops[index].socket);
    zmq_ctx_destroy(pending_ops[index].context);
    
    for (int j = index; j < pending_count - 1; j++) {
        pending_ops[j] = pending_ops[j + 1];
    }
    pending_count--;
}


// Чек отложенных операций (получен ли ответ; не истек ли таймаут)
void check_pending_operations() {
    time_t now = time(NULL); // для подсчета таймаута
    
    for (int i = 0; i < pending_count; i++) {
        PendingOperation* op = &pending_ops[i];
        
        // массив с описанием сокета
        // 0 - не нужен fd; ZMQ_POLLIN - проверяем данные для чтеня;
        // 0 - после опроса сокета он примет маскку событий, произошедших за время опроса
        zmq_pollitem_t items[] = {
            { op->socket, 0, ZMQ_POLLIN, 0 }
        };
        

        // Запрашиваем у сокета наличие входящих данных
        // 1 - запрашиваем один элемент, 0 - неблокирующий режим
        int rc = zmq_poll(items, 1, 0);
        
        if (rc > 0) {  // Если были получены данные
            // Если сообщение получено, то устанавливаем успешный ответ
            if (receive_message(op->socket, &op->msg) > 0) {
                switch (op->type) {
                    case OP_PING:
                        handle_ping_response(1);
                        break;
                    case OP_EXEC:
                        handle_exec_response(op, 1);
                        break;
                    case OP_CREATE:
                        handle_create_response(op, 1);
                        break;
                }
                printf("> ");
                fflush(stdout);
                remove_operation(i);
                i--;
            }
        } 
        // Если истек таймаут, то устанавливаем ответ неуспех
        else if ((now - op->timestamp) * 1000 >= op->timeout_ms) {
            switch (op->type) {
                case OP_PING:
                    handle_ping_response(0);
                    break;
                case OP_EXEC:
                    handle_exec_response(op, 0);
                    break;
                case OP_CREATE:
                    handle_create_response(op, 0);
                    break;
            }
            printf("> ");
            fflush(stdout);
            remove_operation(i);
            i--;
        }
    }
}


// Закрываем сокеты и контексты => зачистка отложенных опреаций
void cleanup_pending_operations() {
    for (int i = 0; i < pending_count; i++) {
        zmq_close(pending_ops[i].socket);
        zmq_ctx_destroy(pending_ops[i].context);
    }
    pending_count = 0;
} 