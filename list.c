#include "list.h"

node *create_node(char *message)
{
    node *new = (node *)malloc(sizeof(node));

    int len = strlen(message);
    new->message = (char* )malloc(len + 1);
    strcpy(new->message, message);
    new->message[len + 1] = 0;
    new->next = NULL;

    return new;
}

void append_node(node **list, char* message)
{
    node* new = create_node(message);

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
    free(node->message);
    free(node);
}

