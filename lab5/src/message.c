#include "../include/message.h"
#include <zmq.h>

// ОТправка сообщения через сокет
int send_message(void* socket, Message* msg) {
    return zmq_send(socket, msg, sizeof(Message), 0);
}

// Получение сообщения 
int receive_message(void* socket, Message* msg) {
    return zmq_recv(socket, msg, sizeof(Message), 0);
} 