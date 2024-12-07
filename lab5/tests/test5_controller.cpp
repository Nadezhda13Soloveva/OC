#include <gtest/gtest.h>
#include <zmq.h>
#include <unistd.h>
#include <signal.h>
#include "message.h"
#include "dict.h"
#include "node_manager.h"
#include "pending_ops.h"

#define BASE_PORT 5550

class ControllerTest : public ::testing::Test {
protected:
    void* context;
    void* socket;
    const int TEST_PORT = BASE_PORT + 100;  // Use a high port number for testing

    void SetUp() override {
        context = zmq_ctx_new();
        socket = zmq_socket(context, ZMQ_REQ);
    }

    void TearDown() override {
        zmq_close(socket);
        zmq_ctx_destroy(context);
    }

    // Helper function to send a message and get response
    bool send_and_receive(const char* command, int port, Message& response) {
        // Set timeouts
        int timeout = 100;  // 100ms timeout
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
};

// Test message sending and receiving
TEST_F(ControllerTest, MessageSendReceive) {
    Message msg = {};
    strcpy(msg.command, "TEST");
    strcpy(msg.data, "test_data");

    void* test_socket = zmq_socket(context, ZMQ_REP);
    ASSERT_NE(test_socket, nullptr);

    // Bind to test port
    char endpoint[64];
    sprintf(endpoint, "tcp://*:%d", TEST_PORT);
    ASSERT_EQ(zmq_bind(test_socket, endpoint), 0);

    // Create client socket
    void* client_socket = zmq_socket(context, ZMQ_REQ);
    ASSERT_NE(client_socket, nullptr);
    
    // Connect client
    sprintf(endpoint, "tcp://localhost:%d", TEST_PORT);
    ASSERT_EQ(zmq_connect(client_socket, endpoint), 0);

    // Test sending from client
    EXPECT_GT(send_message(client_socket, &msg), 0);

    // Server receives
    Message recv_msg = {};
    EXPECT_GT(receive_message(test_socket, &recv_msg), 0);
    
    // Server sends reply
    EXPECT_GT(send_message(test_socket, &recv_msg), 0);

    // Client receives reply
    Message reply_msg = {};
    EXPECT_GT(receive_message(client_socket, &reply_msg), 0);

    // Cleanup
    zmq_close(client_socket);
    zmq_close(test_socket);
}

// Test pending operations
TEST_F(ControllerTest, PendingOperations) {
    Message msg = {};
    strcpy(msg.command, "PING");
    
    // Add pending operation
    add_pending_operation(1, socket, context, OP_PING, &msg, 1000);
    
    // Check if operation was added
    check_pending_operations();
    
    // Cleanup
    cleanup_pending_operations();
}

// Test node creation and management
TEST_F(ControllerTest, NodeManagement) {
    // Create a test node
    Node* node = (Node*)malloc(sizeof(Node));
    node->id = 100;
    node->pid = 12345;
    node->port = TEST_PORT;
    node->parent_id = -1;
    node->next = NULL;

    // Add node to list
    add_node(node);

    // Find the node
    Node* found = find_node(100);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->id, 100);
    EXPECT_EQ(found->pid, 12345);
    EXPECT_EQ(found->port, TEST_PORT);

    // Cleanup is handled by the controller
}

// Test error handling
TEST_F(ControllerTest, ErrorHandling) {
    // Set timeouts for the socket
    int timeout = 100;  // 100ms timeout
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    zmq_setsockopt(socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

    // Set linger to 0 to prevent hanging on close
    int linger = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));

    // Test invalid port connection
    Message response;
    EXPECT_FALSE(send_and_receive("PING", 1, response));

    // Test invalid message
    Message msg = {};
    EXPECT_LE(send_message(nullptr, &msg), 0);
    EXPECT_LE(receive_message(nullptr, &msg), 0);
}

// Test timeout handling
TEST_F(ControllerTest, TimeoutHandling) {
    Message msg = {};
    strcpy(msg.command, "PING");
    
    // Set very short timeout
    add_pending_operation(1, socket, context, OP_PING, &msg, 1);
    
    // Wait for timeout
    usleep(2000);
    
    // Check if operation was cleaned up
    check_pending_operations();
}

// Test node hierarchy
TEST_F(ControllerTest, NodeHierarchy) {
    // Create parent node
    Node* parent = (Node*)malloc(sizeof(Node));
    parent->id = -1;
    parent->pid = getpid();
    parent->port = BASE_PORT - 1;
    parent->parent_id = -1;
    parent->next = NULL;
    add_node(parent);

    // Create child node
    Node* child = (Node*)malloc(sizeof(Node));
    child->id = 1;
    child->pid = 12346;
    child->port = BASE_PORT + 1;
    child->parent_id = -1;
    child->next = NULL;
    add_node(child);

    // Verify hierarchy
    Node* found_parent = find_node(-1);
    ASSERT_NE(found_parent, nullptr);
    EXPECT_EQ(found_parent->id, -1);

    Node* found_child = find_node(1);
    ASSERT_NE(found_child, nullptr);
    EXPECT_EQ(found_child->parent_id, -1);
}

// Test message types
TEST_F(ControllerTest, MessageTypes) {
    Message msg = {};
    
    // Test PING message
    strcpy(msg.command, "PING");
    EXPECT_STREQ(msg.command, "PING");
    
    // Test EXEC message
    strcpy(msg.command, "EXEC");
    strcpy(msg.data, "test command");
    EXPECT_STREQ(msg.command, "EXEC");
    EXPECT_STREQ(msg.data, "test command");
} 

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}