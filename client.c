#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int receiving = 1;
SOCKET connect_socket = INVALID_SOCKET;

void receive_messages(void *);

int main()
{
    WSADATA wsa_data;
    char host_name[DEFAULT_BUFLEN];
    char buffer[DEFAULT_BUFLEN];
    int result;

    // enter localhost for testing
    printf("Enter Host Name: ");
    scanf("%s", &host_name);

    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0)
    {
        printf("WSA startup failed, error %d\n", result);
        return 1;
    }

    struct addrinfo *presult = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = getaddrinfo(host_name, DEFAULT_PORT, &hints, &presult);
    if (result != 0)
    {
        printf("getaddrinfo failed, error %d\n", result);
        WSACleanup();
        return 1;
    }

    connect_socket = INVALID_SOCKET;

    for (ptr = presult; ptr != NULL; ptr = ptr->ai_next)
    {
        connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET)
        {
            printf("socket failed, error %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        printf("Attempting to connect to socket address\n");
        result = connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            printf("connection failed, moving to next address\n");
            closesocket(connect_socket);
            connect_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(presult);

    if (connect_socket == INVALID_SOCKET)
    {
        printf("unable to connect");
        WSACleanup();
        return 1;
    }

    _beginthread(receive_messages, 0, NULL);

    char message[DEFAULT_BUFLEN] = {0};
    while (1)
    {
        // todo: find a better way to flush buffer
        printf(">");
        fgets(message, DEFAULT_BUFLEN, stdin);
        message[strcspn(message, "\n")] = 0;
        if (strcmp(message, "/exit") == 0) {
            receiving = 0;
            break;
        }

        if (strlen(message) != 0)
        {
            result = send(connect_socket, message, strlen(message), 0);
            if (result == SOCKET_ERROR)
            {
                printf("send failed, error %d\n", WSAGetLastError());
                receiving = 0;
                break;
            }
            memset(&message, 0, DEFAULT_BUFLEN);
        }
    }

    printf("Closing socket to server\n");
    closesocket(connect_socket);
    WSACleanup();

    printf("Client closing...\n");
    getc(stdin);

    return 0;
}

void receive_messages(void *ignored)
{
    //printf("receive_messages function called\n");
    char recv_buf[DEFAULT_BUFLEN];
    while (receiving)
    {
        int result = recv(connect_socket, recv_buf, DEFAULT_BUFLEN, 0);
        if (result > 0)
        {
            printf("Message received: %s\n", recv_buf);
            memset(&recv_buf, 0, DEFAULT_BUFLEN);
        }
        else if (result == 0)
        {
            printf("Client receiving connection closing\n");
            break;
        }
    }
    //printf("recieve_messages function exiting\n");
}