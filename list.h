#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <WinSock2.h>

#define DEFAULT_BUFLEN 512

typedef struct node_t
{
    void* item;
    struct node_t *next;
} node;

typedef struct socket_info_t {
    SOCKET socket;
    int in_use;
}socket_info;

node *create_message_node(char* message);
node *create_socket_node(SOCKET socket);
void append_message_node(node **list, char* message);
void append_socket_node(node **list, SOCKET socket);
int delete_socket_node(node **list, SOCKET socket);
void print_socket_list(node *list);
void free_node(node* node);