#define _WIN32_WINNT 0x0501
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void process_connection(SOCKET *listen_socket);

int main()
{

    WSADATA socket_agent_data;

    SOCKET listen_socket = INVALID_SOCKET;

    struct addrinfo *result = NULL, hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];

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

    process_connection(&listen_socket);

    printf("Server closing...\n");
    getc(stdin);
    WSACleanup();

    return 0;
}

void process_connection(SOCKET *listen_socket)
{
    SOCKET client_socket = accept(*listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET)
    {
        printf("accept failed, error %d\n", WSAGetLastError());
        closesocket(*listen_socket);
        WSACleanup();
        return;
    }
    printf("Connection found, accepting...\n");

    printf("Closing socket after connection was accepted\n");
    closesocket(*listen_socket);

    int i_result;
    char recvbuf[DEFAULT_BUFLEN] = {0};
    while (1)
    {
        // todo: find better way to clear buffer
        i_result = recv(client_socket, recvbuf, DEFAULT_BUFLEN, 0);
        if (i_result > 0)
        {
            printf("Message received: %s\n", recvbuf);
            memset(&recvbuf, 0, DEFAULT_BUFLEN);
        }
        else if (i_result == 0)
        {
            printf("Connection Closing\n");
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
