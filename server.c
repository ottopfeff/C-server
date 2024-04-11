#define _WIN32_WINNT 0x0501
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include "list.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define MAX_CONNECTIONS 3

int running = 1;
int connection_count = 0;
SOCKET listen_socket = INVALID_SOCKET;
node *message_list = NULL;
node *client_socket_list = NULL;
SOCKET client_sockets[MAX_CONNECTIONS] = {INVALID_SOCKET};

void manage_connections(void *ignore);
void send_messages(void *ignore);
void process_connection(void *ignore);

int main()
{

    WSADATA socket_agent_data;
    struct addrinfo *result = NULL, hints;
    int iSendResult;

    // init winsock
    printf("Initializing WSA\n");
    int i_result = WSAStartup(MAKEWORD(2, 2), &socket_agent_data);
    if (i_result != 0)
    {
        printf("WSA startup failed, error %d\n", i_result);
        return 1;
    }

    // init addrinfo
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    i_result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (i_result != 0)
    {
        printf("getaddrinfo failed, error %d\n", i_result);
        return 1;
    }

    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET)
    {
        printf("socket failed, error %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    i_result = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (i_result == SOCKET_ERROR)
    {
        printf("bind failed, error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    printf("Listening on port %s\n", DEFAULT_PORT);
    i_result = listen(listen_socket, SOMAXCONN);
    if (i_result == SOCKET_ERROR)
    {
        printf("listen failed, error %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    _beginthread(send_messages, 0, NULL);
    _beginthread(manage_connections, 0, NULL);

    char buffer[DEFAULT_BUFLEN];
    while (running)
    {
        fgets(buffer, DEFAULT_BUFLEN, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "/exit") == 0)
        {
            running = 0;
        }
        if (strcmp(buffer, "/sockets") == 0)
        {
            printf("********************************\n");
            print_socket_list(client_socket_list);
            printf("********************************\n");
        }
        memset(&buffer, 0, DEFAULT_BUFLEN);
    }

    printf("Closing listening socket\n");
    closesocket(listen_socket);

    printf("Closing client sockets\n");
    for (node *socket_node = client_socket_list; socket_node != NULL; socket_node = socket_node->next) {
        socket_info* info = socket_node->item;
        closesocket(info->socket);
    }

    printf("Server closing...\n");
    getc(stdin);
    WSACleanup();

    return 0;
}

void send_messages(void *ignore)
{
    while (running)
    {
        if (message_list != NULL)
        {
            node *message_node = message_list;
            message_list = message_list->next;
            for (node *socket_node = client_socket_list; socket_node != NULL; socket_node = socket_node->next)
            {
                SOCKET *socket_ptr = (SOCKET *)socket_node->item;
                if (socket_ptr != NULL && *socket_ptr != INVALID_SOCKET)
                {
                    int result = send(*socket_ptr, (char *)message_node->item, strlen((char *)message_node->item), 0);

                    if (result == SOCKET_ERROR && running)
                    {
                            printf("send failed, error %d\n", WSAGetLastError());
                    }
                }
            }
            free_node(message_node);
        }
    }
}

void manage_connections(void *ignore)
{
    while (running)
    {
        if (connection_count < MAX_CONNECTIONS)
        {
            SOCKET client_socket = accept(listen_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET)
            {
                if (running)
                    printf("accept failed, error %d\n", WSAGetLastError());
                continue;
            }
            printf("Connection found, accepting...\n");
            append_socket_node(&client_socket_list, client_socket);

            _beginthread(process_connection, 0, NULL);
        }
    }
}

void process_connection(void *ignore)
{
    SOCKET client_socket = INVALID_SOCKET;
    for (node *socket_node = client_socket_list; socket_node != NULL; socket_node = socket_node->next)
    {
        socket_info *socket_info = socket_node->item;
        if (socket_info->in_use)
        {
            continue;
        }
        else
        {
            client_socket = socket_info->socket;
            socket_info->in_use = 1;
        }
    }
    if (client_socket == INVALID_SOCKET)
    {
        printf("process_connection thread could not find a valid socket");
        return;
    }
    connection_count++;

    int i_result;
    char recvbuf[DEFAULT_BUFLEN] = {0};

    while (running)
    {
        // todo: find better way to clear buffer
        i_result = recv(client_socket, recvbuf, DEFAULT_BUFLEN, 0);
        if (i_result > 0)
        {
            printf("Message received: %s\n", recvbuf);
            append_message_node(&message_list, recvbuf);
            memset(&recvbuf, 0, DEFAULT_BUFLEN);
        }
        else if (i_result == 0)
        {
            printf("Client connection Closing\n");
            closesocket(client_socket);
            break;
        }
        else
        {
            if (WSAGetLastError() == 10054)
            {
                printf("Connection closed by client\n");
            }
            else
            {
                printf("recv failed, error %d\n", WSAGetLastError());
            }
            closesocket(client_socket);
            break;
        }
    }
    if (delete_socket_node(&client_socket_list, client_socket))
        ;
    printf("Removed socket from list\n");

    connection_count--;
    return;
}
