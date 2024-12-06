#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"

#define MAX_CMD_LEN 16

#ifdef __cplusplus
extern "C" { // для тестов
#endif

typedef struct {
    char command[MAX_CMD_LEN];
    char data[MAX_MSG_LEN];
} Message;

int send_message(void* socket, Message* msg);
int receive_message(void* socket, Message* msg);

#ifdef __cplusplus
}
#endif

#endif 