#include "../include/message.h"
#include "../include/dict.h"
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>

#define BASE_PORT 5550

Dictionary dictionary; // глобальный словарь

// Команда EXEC
void handle_exec(void* socket, Message* msg) {
    char key[MAX_KEY_LEN];
    char value_str[32];
    int value;
    
    if (sscanf(msg->data, "%s %s", key, value_str) == 2) {
        // Установка значения
        value = atoi(value_str);
        if (dict_set(&dictionary, key, value) == 0) {
            sprintf(msg->data, "Ok");
        } else {
            sprintf(msg->data, "Error: Dictionary is full");
        }
    } else if (sscanf(msg->data, "%s", key) == 1) {
        // Получение значения
        if (dict_get(&dictionary, key, &value) == 0) {
            sprintf(msg->data, "%d", value);
        } else {
            sprintf(msg->data, "'%s' not found", key);
        }
    } else {
        sprintf(msg->data, "Error: Invalid command format");
    }
    // отправляем сообщение обратно
    send_message(socket, msg);
}

// проверка доступности узлов
int ping_node(void* socket) {

    Message msg = {0};
    // создаем сообщение с командой
    strcpy(msg.command, "PING");
    // отправляем сообщение
    if (send_message(socket, &msg) <= 0) {
        return 0;
    }
    // получаем ответ
    if (receive_message(socket, &msg) <= 0) {
        return 0;
    }
    // если 1 значит узел доступен
    return strcmp(msg.data, "1") == 0;
}

// создание узла
void start_compute_node(int id, int parent_id) {
    // инит контекста и сокета
    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REP);
    // строка с адресом для привязки сокета
    char endpoint[64];
    sprintf(endpoint, "tcp://*:%d", BASE_PORT + id);
    // привязываем сокет к адресу
    if (zmq_bind(socket, endpoint) != 0) {
        printf("Failed to bind to %s\n", endpoint);
        exit(1);
    }

    // устанавливаю сокет с неблокирующим приемом с таймаутом
    int timeout = 100;  
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    // время последней проверки и интервал проверки
    time_t last_parent_check = 0;  
    const int PARENT_CHECK_INTERVAL = 0;  

    while (1) {
        time_t now = time(NULL);
        
        // Если указан parent id, проверяем его доступность
        if (parent_id != -1 && now - last_parent_check >= PARENT_CHECK_INTERVAL) {
            // создаем новый сокет и формируем адрес род узла
            void* parent_socket = zmq_socket(context, ZMQ_REQ);
            char parent_endpoint[64];
            sprintf(parent_endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
            
            // настраиваю сокет для проверки родителя с таймаутом
            int parent_timeout = 100;  
            zmq_setsockopt(parent_socket, ZMQ_RCVTIMEO, &parent_timeout, sizeof(parent_timeout));
            zmq_setsockopt(parent_socket, ZMQ_SNDTIMEO, &parent_timeout, sizeof(parent_timeout));

            // проверка подключения и получения ответа от родителя
            if (zmq_connect(parent_socket, parent_endpoint) != 0 || 
                !ping_node(parent_socket)) {
                zmq_close(parent_socket);
                zmq_close(socket);
                zmq_ctx_destroy(context);
                exit(1);
            }
            
            zmq_close(parent_socket);
            last_parent_check = now;
        }

        // ожидаем получения сообщения
        Message msg;
        int rc = receive_message(socket, &msg);
        // если сообщение получено -> чек команды
        if (rc > 0) {
            if (strcmp(msg.command, "EXEC") == 0) {
                handle_exec(socket, &msg);
            } else if (strcmp(msg.command, "PING") == 0) {
                strcpy(msg.data, "1"); // отвечаем
                send_message(socket, &msg);
            }
        //таймаут
        } else if (rc == -1 && errno == EAGAIN) {
            continue;
        }
    }
}

void check_parent_availability(int parent_id) {
    if (parent_id == -1) return; // если род недоступен

    // новый контекст и сокет для проверки доступности род
    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);
    
    int timeout = 1000;
    // установка таймаута получения и отправки сообщений
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
    // формируем адрес род узла
    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
    
    // попытка подключиться к род
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    Message msg = {0};
    strcpy(msg.command, "PING");
    // проверяем доступность
    if (send_message(socket, &msg) <= 0 || 
        receive_message(socket, &msg) <= 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <id> <parent_id>\n", argv[0]);
        return 1;
    }

    int id = atoi(argv[1]);
    int parent_id = atoi(argv[2]);
    start_compute_node(id, parent_id);
    return 0;
}