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

node *create_message_node(char* message);
node *create_socket_node(SOCKET socket);
void append_message_node(node **list, char* message);
void append_socket_node(node **list, SOCKET socket);
void free_node(node* node);