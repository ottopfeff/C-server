#define _WIN32_WINNT NTDDI_VISTA
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <process.h>
#include "list.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define MAX_CONNECTIONS 1000

HANDLE message_lock;
HANDLE socket_lock;

int running = 1;
int connection_count = 0;
SOCKET listen_socket = INVALID_SOCKET;
node *message_list = NULL;
node *client_socket_list = NULL;

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

    printf("Listening on port %s\n", DEFAULT_PORT);
    i_result = listen(listen_socket, SOMAXCONN);
    if (i_result == SOCKET_ERROR)
    {
        printf("listen failed, error %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    message_lock = CreateMutex(NULL, FALSE, NULL);
    socket_lock = CreateMutex(NULL, FALSE, NULL);

    if (!message_lock || !socket_lock) {
        printf("Mutexes were not initialized\n");
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
        else if (strcmp(buffer, "/sockets") == 0)
        {
            if (client_socket_list)
            {
                printf("********************************\n");
                print_socket_list(client_socket_list);
                printf("********************************\n");
            }
            else
            {
                printf("No connections\n");
            }
        }
        else if (strcmp(buffer, "/ip") == 0) //flawed
        {
            struct sockaddr server_info;
            int server_info_length = sizeof(server_info);
            getsockname(listen_socket, &server_info, &server_info_length);
            long unsigned int ipbuflen;
            char ipbuf[50] = {0};
            int res = WSAAddressToStringA(&server_info, 50, NULL, ipbuf, &ipbuflen);
            if (res != 0)
            {
                printf("WSAAddressToStringA failed, error %d\n", WSAGetLastError());
            }
            else
            {
                printf("IP: %s\n", ipbuf);
            }
        }
        memset(&buffer, 0, DEFAULT_BUFLEN);
    }

    printf("Closing listening socket\n");
    closesocket(listen_socket);

    printf("Closing client sockets\n");
    for (node *socket_node = client_socket_list; socket_node != NULL; socket_node = socket_node->next)
    {
        socket_info *info = (socket_info *)socket_node->item;
        closesocket(info->socket);
        // free socket nodes...
    }
    freeaddrinfo(result);

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
            WaitForSingleObject(message_lock, INFINITE);
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
            ReleaseMutex(message_lock);
        }
    }
}

void __cdecl manage_connections(void *ignore)
{
    while (running)
    {
        if (connection_count < MAX_CONNECTIONS)
        {
            // struct sockaddr_in client_info;
            // int addrsize = sizeof(client_info);
            // SOCKET client_socket = accept(listen_socket, (struct sockaddr *)&client_info, &addrsize);
            SOCKET client_socket = accept(listen_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET)
            {
                if (running)
                    printf("accept failed, error %d\n", WSAGetLastError());
                continue;
            }

            WaitForSingleObject(socket_lock, INFINITE);
            append_socket_node(&client_socket_list, client_socket);
            ReleaseMutex(socket_lock);

            _beginthread(process_connection, 0, NULL);
        }
    }
}

void process_connection(void *ignore)
{
    SOCKET client_socket = INVALID_SOCKET;
    for (node *socket_node = client_socket_list; socket_node != NULL; socket_node = socket_node->next)
    {
        socket_info *info = (socket_info *)socket_node->item;
        if (info->in_use)
        {
            continue;
        }
        else
        {
            client_socket = info->socket;
            info->in_use = 1;
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

            WaitForSingleObject(message_lock, INFINITE);
            append_message_node(&message_list, recvbuf);
            ReleaseMutex(message_lock);

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

    WaitForSingleObject(socket_lock, INFINITE);
    if (delete_socket_node(&client_socket_list, client_socket))
        printf("Removed socket from list\n");
    ReleaseMutex(socket_lock);

    connection_count--;
    return;
}
