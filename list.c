#include "list.h"

node *create_message_node(char *message)
{
    node *new = (node *)malloc(sizeof(node));

    int len = strlen(message);
    char *new_message = (char *)malloc(len + 1);
    strcpy(new_message, message);
    new_message[len + 1] = 0;

    new->item = new_message;
    new->next = NULL;

    return new;
}

node *create_socket_node(SOCKET socket)
{
    node *new = (node *)malloc(sizeof(node));

    socket_info *socket_info = malloc(sizeof(socket_info));
    socket_info->socket = socket;
    socket_info->in_use = 0;

    new->item = socket_info;
    new->next = NULL;

    return new;
}

void append_message_node(node **list, char *message)
{
    node *new = create_message_node((void *)message);

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

void append_socket_node(node **list, SOCKET socket)
{
    node *new = create_socket_node(socket);

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

int delete_socket_node(node **list, SOCKET socket)
{
    node *curr = *list;
    node *prev = NULL;

    while (curr != NULL)
    {
        socket_info *info = (socket_info *)curr->item;
        if (info->socket = socket)
        {
            if (prev == NULL)
                *list = curr->next;
            else
                prev->next = curr->next;
            free_node(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    printf("failed to find node");
    return 0;
}

void print_socket_list(node *list) {
    node* curr = list;
    
    while (curr) {
        socket_info *info = (socket_info *)curr->item;
        printf("SOCKET %d, IN_USE: %d\n", info->socket, info->in_use);
        curr = curr->next;
    }
}

void free_node(node *node)
{
    free(node->item);
    free(node);
}
