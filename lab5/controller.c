#include "../include/message.h"
#include "../include/pending_ops.h"
#include "../include/node_manager.h"
#include <sys/wait.h>
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

#define BASE_PORT 5550
#define MAX_NODES 100

// создаем корневой узел
int create_root_node(void) {
    Node* new_node = malloc(sizeof(Node));
    new_node->id = -1;
    new_node->pid = getpid();  
    new_node->port = BASE_PORT - 1;
    new_node->parent_id = -1;
    new_node->next = NULL;
    
    add_node(new_node);
    return 0;
}

// CREATE
void create_node(int id, int parent_id) {
    // проверка что такого пока нет
    if (find_node(id) != NULL) {
        printf("Error: Already exists\n");
        return;
    }

    // дефолтный parent id
    if (parent_id == -1 || parent_id == 0) {
        parent_id = -1;
    }

    // чек что род существует
    Node* parent = find_node(parent_id);
    if (!parent) {
        printf("> Error: Parent not found\n");
        return;
    }

    // если задан родитель
    if (parent_id != -1) {
        // задаем контекст и сокет в режиме запрос-ответ
        void* context = zmq_ctx_new();
        void* socket = zmq_socket(context, ZMQ_REQ);
        
        int timeout = 100;
        // устанавливаем таймаут для отправки и получения ответа
        zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

        // задаем  время ожидания отправки оставшихся сообщений перед закрытием
        int linger = 0;
        zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
        // адрес род
        char endpoint[64];
        sprintf(endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
        // попытка подключиться к род
        if (zmq_connect(socket, endpoint) != 0) {
            zmq_close(socket);
            zmq_ctx_destroy(context);
            printf("> Error: Parent is unavailable\n");
            return;
        }
        
        Message msg = {0};
        strcpy(msg.command, "PING");
        // проверка доступности родителя
        if (send_message(socket, &msg) <= 0 || receive_message(socket, &msg) <= 0) {
            zmq_close(socket);
            zmq_ctx_destroy(context);
            printf("> Error: Parent is unavailable\n");
            return;
        }

        zmq_close(socket);
        zmq_ctx_destroy(context);
    }

    // создаем дочерний
    pid_t pid = fork();
    if (pid == 0) {
        // в дочернем
        char id_str[16], parent_str[16];
        sprintf(id_str, "%d", id);
        sprintf(parent_str, "%d", parent_id);
        // запускаем исполнителя
        if (execl("./worker", "worker", id_str, parent_str, NULL) == -1) {
            printf("> Error: Failed to start worker: %s\n", strerror(errno));
            exit(1);
        }
        exit(1);
    } else if (pid > 0) {
        // в родительском
        usleep(100000);
        
        int status;
        // ожидаем дочерний
        pid_t result = waitpid(pid, &status, WNOHANG);
        // успешно создан дочерний
        if (result == 0) {  
            Node* new_node = malloc(sizeof(Node));
            new_node->id = id;
            new_node->pid = pid;
            new_node->port = BASE_PORT + id;
            new_node->parent_id = parent_id;
            new_node->next = NULL;
            
            add_node(new_node);
            printf("> Ok: %d\n", pid);
        } else {
            printf("> Error: Worker failed to start\n");
        }
    } else {
        printf("> Error: Failed to create process\n");
    }
}

// EXEC
int exec_command(int id, const char* command) {
    Node* node = find_node(id);
    // проверка существования узла
    if (!node) {
        printf("> Error:%d: Not found\n", id);
        return -1;
    }

    // задаем контекст и сокет
    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);

    int timeout = 100;  
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", node->port);
    // проверка подключиться к узлу
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Error:%d: Node is unavailable\n", id);
        return -1;
    }
    // формирование и отправка сообщения
    Message msg = {0};
    strcpy(msg.command, "EXEC");
    strncpy(msg.data, command, MAX_MSG_LEN - 1);
    msg.data[MAX_MSG_LEN - 1] = '\0';
    // удалось ли отправить
    if (send_message(socket, &msg) <= 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Error:%d: Node is unavailable\n", id);
        return -1;
    }

    // добавляем операцию
    add_pending_operation(id, socket, context, OP_EXEC, &msg, 1000);
    return 0;
}

// PING
int ping_command(int id) {
    Node* node = find_node(id);
    if (!node) {
        printf("> Error: Not found\n");
        return -1;
    }
    // задаем сокет и контекст
    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);

    int timeout = 100;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", node->port);
    // прверка подсоединения
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Ok: 0\n");
        return -1;
    }

    Message msg = {0};
    strcpy(msg.command, "PING");
    // не удалось отправить
    if (send_message(socket, &msg) <= 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Ok: 0\n");
        return -1;
    }
    // добавляем операцию
    add_pending_operation(id, socket, context, OP_PING, &msg, 1000);
    return 0;
}

// KILL
void kill_node(int pid) {
    // удачно удалился
    if (kill(pid, SIGTERM) == 0) {
        printf("> Ok: %d killed\n", pid);
    } else {
        perror("Error");
    }
}

// чек ввода
int check_user_input(char* buffer, size_t size) {
    static int prompt_shown = 0; // флаг отслеживания >
    struct pollfd fds; // структурка для опроса fd
    fds.fd = STDIN_FILENO; // станд ввод
    fds.events = POLLIN; // наличие входных данных (чтение)

    int ret = poll(&fds, 1, 0);  // неблокирующий опрос fd на наличие событий
    // есть входные данные
    if (ret > 0 && (fds.revents & POLLIN)) {
        if (fgets(buffer, size, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;  
            prompt_shown = 0;  // сброс флага
            return 1;
        }
    // ввода нет и нет >
    } else if (!prompt_shown) {
        printf("> ");
        fflush(stdout);
        prompt_shown = 1; // чтобы не выводить > повторно
    }
    return 0;
}

int main() {
    // создаем контекст, команду и части команды
    void* context = zmq_ctx_new();
    char command[MAX_MSG_LEN];
    char cmd_type[32], param1[32], param2[32], param3[32];

    // создаем корневой узел
    if (create_root_node() != 0) {
        fprintf(stderr, "Failed to create root node\n");
        return 1;
    }

    while (1) {
        // если есть ввод 
        if (check_user_input(command, MAX_MSG_LEN)) {
            // разбиваем команду
            int params = sscanf(command, "%s %s %s %s", cmd_type, param1, param2, param3);
            
            if (strcmp(cmd_type, "create") == 0) {
                if (params == 3) {
                    create_node(atoi(param1), atoi(param2));
                } else if (params == 2) {
                    create_node(atoi(param1), -1);  
                }
            }
            else if (strcmp(cmd_type, "exec") == 0 && params >= 3) {
                char* cmd_start = strstr(command, param2); // указатель на начало подстроки
                exec_command(atoi(param1), cmd_start);
            }
            else if (strcmp(cmd_type, "ping") == 0 && params == 2) {
                ping_command(atoi(param1));
            }
            else if (strcmp(cmd_type, "kill") == 0 && params == 2) {
                kill_node(atoi(param1));
            }
            else if (strcmp(cmd_type, "exit") == 0) {
                //  зачистка дочерних
                Node* current = head;
                while (current != NULL) {
                    if (current->id != -1) {
                        printf("Terminating worker with PID: %d\n", current->pid);
                        kill(current->pid, SIGTERM);
                    }
                    current = current->next;
                }
                break;
            }
        }
        // проверка отложенных операций
        check_pending_operations();
        usleep(1000); 
    }
    // очистка отложенных операций и контекста
    cleanup_pending_operations();
    zmq_ctx_destroy(context);
    return 0;
}