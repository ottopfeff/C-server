#include "list.h"

node *create_message_node(char *message)
{
    node *new = (node *)malloc(sizeof(node));

    int len = strlen(message);
    char* new_message = (char* )malloc(len + 1);
    strcpy(new_message, message);
    new_message[len + 1] = 0;
    
    new->item = new_message;
    new->next = NULL;

    return new;
}

node *create_socket_node(SOCKET socket) {
    node *new = (node *)malloc(sizeof(node));

    SOCKET* new_socket = (SOCKET* )malloc(sizeof(SOCKET));
    *new_socket = socket;

    new->item = new_socket;
    new->next = NULL;

    return new;
}

void append_message_node(node **list, char* message)
{
    node* new = create_message_node((void* )message);

    node *curr = *list;
    if (curr == NULL)
    {
        *list = new;
    }
    while (curr)
    {
        if (curr->next == NULL)
        {
            curr->next = new;
            return;
        }
        else
        {
            curr = curr->next;
        }
    }
}

void append_socket_node(node **list, SOCKET message) {
        node* new = create_socket_node((void* )socket);

    node *curr = *list;
    if (curr == NULL)
    {
        *list = new;
    }
    while (curr)
    {
        if (curr->next == NULL)
        {
            curr->next = new;
            return;
        }
        else
        {
            curr = curr->next;
        }
    }
}

void free_node(node* node) {
    free(node->item);
    free(node);
}

