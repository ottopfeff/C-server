#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <WinSock2.h>

#define DEFAULT_BUFLEN 512

typedef struct node_t
{
    char* message;
    struct node_t *next;
} node;

node *create_node(char* message);
void append_node(node **list, char* message);
void free_node(node* node);