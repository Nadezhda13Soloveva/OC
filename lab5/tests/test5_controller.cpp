#include <gtest/gtest.h>
#include <zmq.h>
#include <unistd.h>
#include <signal.h>
#include "message.h"
#include "dict.h"
#include "node_manager.h"
#include "pending_ops.h"

#define BASE_PORT 5550
#define TEST_PORT (BASE_PORT + 100)

bool send_and_receive(void* socket, const char* command, int port, Message& response) {
    int timeout = 100;  
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
    
    char endpoint[64];
    sprintf(endpoint, "tcp://localhost:%d", port);
    
    if (zmq_connect(socket, endpoint) != 0) {
        return false;
    }

    Message msg = {};
    strcpy(msg.command, command);
    
    if (send_message(socket, &msg) <= 0) {
        zmq_disconnect(socket, endpoint);
        return false;
    }

    if (receive_message(socket, &response) <= 0) {
        zmq_disconnect(socket, endpoint);
        return false;
    }

    zmq_disconnect(socket, endpoint);
    return true;
}

// Чек отправки и получения сообщений
TEST(ControllerTest, MessageSendReceive) {
    
    void* context = zmq_ctx_new();
    ASSERT_NE(context, nullptr);

    void* test_socket = zmq_socket(context, ZMQ_REP);
    ASSERT_NE(test_socket, nullptr);

    char endpoint[64];
    sprintf(endpoint, "tcp://*:%d", TEST_PORT);
    ASSERT_EQ(zmq_bind(test_socket, endpoint), 0);

    void* client_socket = zmq_socket(context, ZMQ_REQ);
    ASSERT_NE(client_socket, nullptr);
    sprintf(endpoint, "tcp://localhost:%d", TEST_PORT);
    ASSERT_EQ(zmq_connect(client_socket, endpoint), 0);

    Message msg = {};
    strcpy(msg.command, "TEST");
    strcpy(msg.data, "test_data");

    EXPECT_GT(send_message(client_socket, &msg), 0);

    Message recv_msg = {};
    EXPECT_GT(receive_message(test_socket, &recv_msg), 0);
    EXPECT_GT(send_message(test_socket, &recv_msg), 0);

    Message reply_msg = {};
    EXPECT_GT(receive_message(client_socket, &reply_msg), 0);

    EXPECT_STREQ(recv_msg.command, "TEST");
    EXPECT_STREQ(recv_msg.data, "test_data");

    zmq_close(client_socket);
    zmq_close(test_socket);
    zmq_ctx_destroy(context);
}

// Чек добавления и обработки отложенных операций
TEST(ControllerTest, PendingOperations) {
    
    void* context = zmq_ctx_new();
    ASSERT_NE(context, nullptr);

    void* socket = zmq_socket(context, ZMQ_REQ);
    ASSERT_NE(socket, nullptr);

    Message msg = {};
    strcpy(msg.command, "PING");

    add_pending_operation(1, socket, context, OP_PING, &msg, 1000);
    check_pending_operations();

    cleanup_pending_operations();
    zmq_close(socket);
    zmq_ctx_destroy(context);
}


// Чек создания, добавления, поиска и правильности атрибутов узла
TEST(ControllerTest, NodeManagement) {
    
    Node* node = (Node*)malloc(sizeof(Node));
    node->id = 100;
    node->pid = 12345;
    node->port = TEST_PORT;
    node->parent_id = -1;
    node->next = NULL;

    add_node(node);
    Node* found = find_node(100);

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->id, 100);
    EXPECT_EQ(found->pid, 12345);
    EXPECT_EQ(found->port, TEST_PORT);
}


// Чек обработки ошибок неправильных операций
TEST(ControllerTest, ErrorHandling) {
    
    void* context = zmq_ctx_new();
    ASSERT_NE(context, nullptr);

    void* socket = zmq_socket(context, ZMQ_REQ);
    ASSERT_NE(socket, nullptr);

    int timeout = 100;
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    Message response;

    EXPECT_FALSE(send_and_receive(socket, "PING", 1, response));

    Message msg = {};
    EXPECT_LE(send_message(nullptr, &msg), 0);
    EXPECT_LE(receive_message(nullptr, &msg), 0);

    zmq_close(socket);
    zmq_ctx_destroy(context);
}


// Чек поведения при таймауте
TEST(ControllerTest, TimeoutHandling) {
    
    void* context = zmq_ctx_new();
    ASSERT_NE(context, nullptr);

    void* socket = zmq_socket(context, ZMQ_REQ);
    ASSERT_NE(socket, nullptr);

    Message msg = {};
    strcpy(msg.command, "PING");
    
    add_pending_operation(1, socket, context, OP_PING, &msg, 1);
    usleep(2000);
    check_pending_operations();

    cleanup_pending_operations();
    zmq_close(socket);
    zmq_ctx_destroy(context);
}


// Чек создания и сохранения иерархической связи между род и дочерним
TEST(ControllerTest, NodeHierarchy) {
    
    Node* parent = (Node*)malloc(sizeof(Node));
    parent->id = -1;
    parent->pid = getpid();
    parent->port = BASE_PORT - 1;
    parent->parent_id = -1;
    parent->next = NULL;
    add_node(parent);

    Node* child = (Node*)malloc(sizeof(Node));
    child->id = 1;
    child->pid = 12346;
    child->port = BASE_PORT + 1;
    child->parent_id = -1;
    child->next = NULL;
    add_node(child);

    Node* found_parent = find_node(-1);
    Node* found_child = find_node(1);

    ASSERT_NE(found_parent, nullptr);
    EXPECT_EQ(found_parent->id, -1);
    ASSERT_NE(found_child, nullptr);
    EXPECT_EQ(found_child->parent_id, -1);

}


// Чек установки и сохранения сообщений с разными командами
TEST(ControllerTest, MessageTypes) {
    
    Message msg = {};
    
    strcpy(msg.command, "PING");
    EXPECT_STREQ(msg.command, "PING");
    
    strcpy(msg.command, "EXEC");
    strcpy(msg.data, "test command");
    EXPECT_STREQ(msg.command, "EXEC");
    EXPECT_STREQ(msg.data, "test command");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
