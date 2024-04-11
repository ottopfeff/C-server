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
SOCKET listen_socket = INVALID_SOCKET;
node *message_list;
SOCKET client_sockets[MAX_CONNECTIONS] = {INVALID_SOCKET};

void process_connection(void *ignore);
void accept_connections(void *ignore);
void send_messages(void *ignore);

int main()
{

    WSADATA socket_agent_data;
    struct addrinfo *result = NULL, hints;
    int iSendResult;
    int connection_count = 0;

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

    if (connection_count < MAX_CONNECTIONS)
    {
        process_connection(NULL);
    }
    else
    {
        printf("Maximum number of connections has already been met\n");
    }

    printf("Closing listening socket\n");
    closesocket(listen_socket);

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
            node* message_node = message_list;
            message_list = message_list->next;
            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                if (client_sockets[i] != INVALID_SOCKET)
                {
                    int result = send(client_sockets[i], (char*)message_node->item, strlen((char*)message_node->item), 0);

                    if (result == SOCKET_ERROR)
                    {
                        printf("send failed, error %d\n", WSAGetLastError());
                    }
                }
            }
            free_node(message_node);
        }
    }
}

void process_connection(void *ignore)
{
    SOCKET client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET)
    {
        printf("accept failed, error %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return;
    }
    printf("Connection found, accepting...\n");
    client_sockets[0] = client_socket;

    int i_result;
    char recvbuf[DEFAULT_BUFLEN] = {0};

    // printf("Sending test message to client\n");
    // strcpy(recvbuf, "from server");
    // send(client_socket, recvbuf, DEFAULT_BUFLEN, 0);
    // memset(&recvbuf, 0, DEFAULT_BUFLEN);

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
    return;
}
