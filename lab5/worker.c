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

DictEntry dictionary[MAX_DICT_ENTRIES];

void init_dictionary() {
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        dictionary[i].is_set = 0;
    }
}

int set_value(const char* key, int value) {
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        if (dictionary[i].is_set && strcmp(dictionary[i].key, key) == 0) {
            dictionary[i].value = value;
            return 0;
        }
    }
    
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        if (!dictionary[i].is_set) {
            strncpy(dictionary[i].key, key, MAX_KEY_LEN - 1);
            dictionary[i].key[MAX_KEY_LEN - 1] = '\0';
            dictionary[i].value = value;
            dictionary[i].is_set = 1;
            return 0;
        }
    }
    return -1;
}

int get_value(const char* key, int* value) {
    for (int i = 0; i < MAX_DICT_ENTRIES; i++) {
        if (dictionary[i].is_set && strcmp(dictionary[i].key, key) == 0) {
            *value = dictionary[i].value;
            return 0;
        }
    }
    return -1;
}

void handle_exec(void* socket, Message* msg) {
    char key[MAX_KEY_LEN];
    char value_str[32];
    int value;
    
    if (sscanf(msg->data, "%s %s", key, value_str) == 2) {
        // Установка значения
        value = atoi(value_str);
        if (set_value(key, value) == 0) {
            sprintf(msg->data, "Ok");
        } else {
            sprintf(msg->data, "Error: Dictionary is full");
        }
    } else if (sscanf(msg->data, "%s", key) == 1) {
        // Получение значения
        if (get_value(key, &value) == 0) {
            sprintf(msg->data, "%d", value);
        } else {
            sprintf(msg->data, "'%s' not found", key);
        }
    } else {
        sprintf(msg->data, "Error: Invalid command format");
    }
    
    send_message(socket, msg);
}

int ping_node(void* socket) {
    Message msg = {0};
    strcpy(msg.command, "PING");
    
    if (send_message(socket, &msg) <= 0) {
        return 0;
    }
    
    if (receive_message(socket, &msg) <= 0) {
        return 0;
    }
    
    return strcmp(msg.data, "1") == 0;
}

void start_compute_node(int id, int parent_id) {
    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REP);
    
    char endpoint[64];
    sprintf(endpoint, "tcp://*:%d", BASE_PORT + id);
    
    if (zmq_bind(socket, endpoint) != 0) {
        printf("Failed to bind to %s\n", endpoint);
        exit(1);
    }

    // Set non-blocking receive with short timeout
    int timeout = 100;  // 100ms timeout
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    time_t last_parent_check = 0;  // Start with immediate parent check
    const int PARENT_CHECK_INTERVAL = 0;  // Check as often as possible

    while (1) {
        time_t now = time(NULL);
        
        // Check parent more frequently
        if (parent_id != -1 && now - last_parent_check >= PARENT_CHECK_INTERVAL) {
            void* parent_socket = zmq_socket(context, ZMQ_REQ);
            char parent_endpoint[64];
            sprintf(parent_endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
            
            int parent_timeout = 100;  // Short timeout for parent check
            zmq_setsockopt(parent_socket, ZMQ_RCVTIMEO, &parent_timeout, sizeof(parent_timeout));
            zmq_setsockopt(parent_socket, ZMQ_SNDTIMEO, &parent_timeout, sizeof(parent_timeout));

            if (zmq_connect(parent_socket, parent_endpoint) != 0 || 
                !ping_node(parent_socket)) {
                // printf("Parent node %d is unavailable, terminating...\n", parent_id);
                zmq_close(parent_socket);
                zmq_close(socket);
                zmq_ctx_destroy(context);
                exit(1);
            }
            
            zmq_close(parent_socket);
            last_parent_check = now;
        }

        Message msg;
        int rc = receive_message(socket, &msg);
        if (rc > 0) {
            if (strcmp(msg.command, "EXEC") == 0) {
                handle_exec(socket, &msg);
            } else if (strcmp(msg.command, "PING") == 0) {
                strcpy(msg.data, "1");
                send_message(socket, &msg);
            }
        } else if (rc == -1 && errno == EAGAIN) {
            // Timeout, continue to check parent
            continue;
        }
    }
}

void check_parent_availability(int parent_id) {
    if (parent_id == -1) return;

    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);
    
    int timeout = 1000;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
    
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        exit(1);
    }

    Message msg = {0};
    strcpy(msg.command, "PING");
    
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