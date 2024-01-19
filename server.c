#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DEFAULT_PORT "27015"
#define IP "192.168.1.9"
//#define IP "172.29.80.1"

int main()
{
    WSADATA socket_data;

    int res;
    if (res = WSAStartup(MAKEWORD(2, 2), &socket_data) != 0)
    {
        printf("WSA startup failed\n");
        return 1;
    }
    printf("%s\n", socket_data.szSystemStatus);
    printf("%s\n", socket_data.szDescription);

    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int addr_res = getaddrinfo(IP, DEFAULT_PORT, &hints, &result);
    if (addr_res != 0)
    {
        printf("getaddrinfo failed\n");
        WSACleanup();
        return 1;
    }
    SOCKET connect_socket = INVALID_SOCKET;

    ptr = result;
    connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (connect_socket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    return 0;
}