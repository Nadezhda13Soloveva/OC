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

// Function to create the root node (returns 0 on success)
int create_root_node(void) {
    Node* new_node = malloc(sizeof(Node));
    new_node->id = -1;
    new_node->pid = getpid();  // Controller's PID
    new_node->port = BASE_PORT - 1;  // Special port for root
    new_node->parent_id = -1;
    new_node->next = NULL;
    
    add_node(new_node);
    return 0;
}

void create_node(int id, int parent_id) {
    if (find_node(id) != NULL) {
        printf("Error: Already exists\n");
        return;
    }

    // If no parent specified, use root (-1)
    if (parent_id == -1 || parent_id == 0) {
        parent_id = -1;
    }

    // Check parent exists
    Node* parent = find_node(parent_id);
    if (!parent) {
        printf("> Error: Parent not found\n");
        return;
    }

    // For non-root nodes, check if parent is available
    if (parent_id != -1) {
        void* context = zmq_ctx_new();
        void* socket = zmq_socket(context, ZMQ_REQ);
        
        int timeout = 100;
        zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

        int linger = 0;
        zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

        char endpoint[64];
        sprintf(endpoint, "tcp://localhost:%d", BASE_PORT + parent_id);
        
        if (zmq_connect(socket, endpoint) != 0) {
            zmq_close(socket);
            zmq_ctx_destroy(context);
            printf("> Error: Parent is unavailable\n");
            return;
        }

        Message msg = {0};
        strcpy(msg.command, "PING");
        
        if (send_message(socket, &msg) <= 0 || receive_message(socket, &msg) <= 0) {
            zmq_close(socket);
            zmq_ctx_destroy(context);
            printf("> Error: Parent is unavailable\n");
            return;
        }

        zmq_close(socket);
        zmq_ctx_destroy(context);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char id_str[16], parent_str[16];
        sprintf(id_str, "%d", id);
        sprintf(parent_str, "%d", parent_id);
        
        if (execl("./worker", "worker", id_str, parent_str, NULL) == -1) {
            printf("> Error: Failed to start worker: %s\n", strerror(errno));
            exit(1);
        }
        exit(1);
    } else if (pid > 0) {
        // Parent process
        usleep(100000);  // 100ms delay to ensure process starts
        
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) {  // Process started successfully
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

int exec_command(int id, const char* command) {
    Node* node = find_node(id);
    if (!node) {
        printf("> Error:%d: Not found\n", id);
        return -1;
    }

    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);

    int timeout = 100;  // Reduced timeout
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", node->port);
    
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Error:%d: Node is unavailable\n", id);
        return -1;
    }

    Message msg = {0};
    strcpy(msg.command, "EXEC");
    strncpy(msg.data, command, MAX_MSG_LEN - 1);
    msg.data[MAX_MSG_LEN - 1] = '\0';
    
    if (send_message(socket, &msg) <= 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Error:%d: Node is unavailable\n", id);
        return -1;
    }

    // Add to pending operations
    add_pending_operation(id, socket, context, OP_EXEC, &msg, 1000);
    return 0;
}

int ping_command(int id) {
    Node* node = find_node(id);
    if (!node) {
        printf("> Error: Not found\n");
        return -1;
    }

    void* context = zmq_ctx_new();
    void* socket = zmq_socket(context, ZMQ_REQ);

    int timeout = 100;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", node->port);
    
    if (zmq_connect(socket, endpoint) != 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Ok: 0\n");
        return -1;
    }

    Message msg = {0};
    strcpy(msg.command, "PING");
    
    if (send_message(socket, &msg) <= 0) {
        zmq_close(socket);
        zmq_ctx_destroy(context);
        printf("> Ok: 0\n");
        return -1;
    }

    add_pending_operation(id, socket, context, OP_PING, &msg, 1000);
    return 0;
}

// Function to kill a node by its process ID
void kill_node(int pid) {
    if (kill(pid, SIGTERM) == 0) {
        printf("> Ok: %d killed\n", pid);
    } else {
        perror("Error");
    }
}

// Function to check for user input
int check_user_input(char* buffer, size_t size) {
    static int prompt_shown = 0;
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;

    int ret = poll(&fds, 1, 0);  // Non-blocking poll
    if (ret > 0 && (fds.revents & POLLIN)) {
        if (fgets(buffer, size, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;  // Remove newline
            prompt_shown = 0;  // Reset prompt flag after input
            return 1;
        }
    } else if (!prompt_shown) {
        printf("> ");
        fflush(stdout);
        prompt_shown = 1;
    }
    return 0;
}

int main() {
    void* context = zmq_ctx_new();
    char command[MAX_MSG_LEN];
    char cmd_type[32], param1[32], param2[32], param3[32];

    // Create root node at startup
    if (create_root_node() != 0) {
        fprintf(stderr, "Failed to create root node\n");
        return 1;
    }

    while (1) {
        // Check for user input
        if (check_user_input(command, MAX_MSG_LEN)) {
            int params = sscanf(command, "%s %s %s %s", cmd_type, param1, param2, param3);
            
            if (strcmp(cmd_type, "create") == 0) {
                if (params == 3) {
                    create_node(atoi(param1), atoi(param2));
                } else if (params == 2) {
                    create_node(atoi(param1), -1);  // Default to root
                }
            }
            else if (strcmp(cmd_type, "exec") == 0 && params >= 3) {
                char* cmd_start = strstr(command, param2);
                exec_command(atoi(param1), cmd_start);
            }
            else if (strcmp(cmd_type, "ping") == 0 && params == 2) {
                ping_command(atoi(param1));
            }
            else if (strcmp(cmd_type, "kill") == 0 && params == 2) {
                kill_node(atoi(param1));
            }
            else if (strcmp(cmd_type, "exit") == 0) {
                break;
            }
        }

        // Check pending operations
        check_pending_operations();
        usleep(1000);  // Small sleep to prevent CPU hogging
    }

    cleanup_pending_operations();
    zmq_ctx_destroy(context);
    return 0;
}