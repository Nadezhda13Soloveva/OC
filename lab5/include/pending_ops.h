#ifndef PENDING_OPS_H
#define PENDING_OPS_H

#include <time.h>
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

// Типы операций
typedef enum {
    OP_PING,
    OP_EXEC,
    OP_CREATE
} OperationType;


// Структурка отложенной операции
typedef struct {
    int id;
    void* socket; // сокет и контекст для ZeroMQ
    void* context;
    OperationType type;
    Message msg;
    time_t timestamp; // метка добавления
    int timeout_ms; 
} PendingOperation;

void add_pending_operation(int id, void* socket, void* context, OperationType type, Message* msg, int timeout_ms);
void check_pending_operations(void);
void cleanup_pending_operations(void);

#ifdef __cplusplus
}
#endif

#endif 