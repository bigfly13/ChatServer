#include <winsock2.h>
#include<Windows.h>
#include<ws2tcpip.h>
#include<stdio.h>

#pragma comment(lib, "ws2_32.lib")  // 链接 Winsock 库
#define BUF_SIZE 1024

char sdMsg[1024]; 
char rcMsg[1024];

unsigned SendMsg(void *arg)
{
    SOCKET sock = *((SOCKET*)arg);
    while (1)
    {
        scanf("%s", sdMsg);
        if (!strcmp(sdMsg, "QUIT\n") || !strcmp(sdMsg, "quit\n"))
        {
            closesocket(sock);
            exit(0);
        }

        send(sock, sdMsg, strlen(sdMsg), 0);
    }
    return 0;
}

unsigned RecvMsg(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);
    
    while (1)
    {
        int len = recv(sock, rcMsg, sizeof(rcMsg), 0);
        if (len == -1)
        {
            closesocket(sock);
            return -1;
        }
        rcMsg[len] = '\0';
        printf("%s\n", rcMsg);
    }
    return 0;
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // 初始化失败
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "8.148.65.28", &addr.sin_addr);
    addr.sin_port = htons(9999);

    int ret = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
    if (ret != 0)
    {
        int err = WSAGetLastError();
        printf("connect failed: %d\n", err);
    }
    printf("欢迎来到多人聊天室，请输入用户名：");

    HANDLE hSendHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendMsg, (void*)&sock, 0, NULL);
    HANDLE hRecvHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvMsg, (void*)&sock, 0, NULL);

    WaitForSingleObject(hSendHand, INFINITE);
    WaitForSingleObject(hRecvHand, INFINITE);

    closesocket(sock);
    WSACleanup();
    return 0;
}