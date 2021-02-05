#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

int main(const int, const char* const* const){
    const int bufferSize = 1024;
    const int portNumber = 7890;
    char IPAddress[16] = "127.0.0.1";
    WSAData wsaData{};
    SOCKET mySocket = 0;
    SOCKADDR_IN serverAddress{};
    fd_set readFDS{};
    fd_set tempFDS{};
    TIMEVAL timeout{};
    char msgBuffer[bufferSize]{};
    int msgLen = 0;
    int result = 0;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        (void)printf("WSAStartup() error!");
        return 1;
    }

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = inet_addr(IPAddress);

    result = connect(mySocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if(result == SOCKET_ERROR){
        (void)closesocket(mySocket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        (void)WSACleanup();
        return 1;
    }

    FD_ZERO(&readFDS);
    FD_SET(mySocket, &readFDS);

    (void)printf("[I/O multiplexing client] Enter msg: ");
    memset(msgBuffer, '\0', bufferSize);

    for(;;){
        if(_kbhit()){
            const int myCh = _getch();
            msgBuffer[msgLen] = myCh;

            if(msgBuffer[msgLen] == '\n' || msgBuffer[msgLen] == '\r'){
                (void)putchar('\n');
                msgBuffer[msgLen++] = '\0';

                result = send(mySocket, msgBuffer, msgLen, 0);
                if(result == SOCKET_ERROR){
                    (void)printf("send failed with error %d\n", WSAGetLastError());
                    (void)closesocket(mySocket);
                    (void)WSACleanup();
                    return 1;
                }
                (void)printf("\"%s\" (to %d.%d.%d.%d: %d, bytes sent: %d)\n",
                    msgBuffer,
                    serverAddress.sin_addr.S_un.S_un_b.s_b1,
                    serverAddress.sin_addr.S_un.S_un_b.s_b2,
                    serverAddress.sin_addr.S_un.S_un_b.s_b3,
                    serverAddress.sin_addr.S_un.S_un_b.s_b4,
                    ntohs(serverAddress.sin_port),
                    result
                );

                msgLen = 0;
                memset(msgBuffer, '\0', bufferSize);
            } else{
                if(myCh != (int)'\b'){
                    (void)putchar(msgBuffer[msgLen++]);
                } else if(msgLen > 0){
                    (void)printf("\b");
                    (void)putchar('\0');
                    (void)putchar('\b');
                    --msgLen;
                }
            }
        } else{
            tempFDS = readFDS;
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000;

            if((result = select(0, &tempFDS, 0, 0, &timeout)) < 0){
                (void)closesocket(mySocket);
                (void)printf("select() error\n");
                return 1;
            } else if(result > 0){
                memset(msgBuffer, '\0', bufferSize);

                result = recv(mySocket, msgBuffer, bufferSize, 0);
                if(result < 0){
                    (void)closesocket(mySocket);
                    (void)printf("Exceptional error: Socket Handle [%llu]\n", mySocket);
                    return 1;
                } else if(result == 0){ //Connection closed, msg has arrived
                    (void)closesocket(mySocket);
                    (void)printf("Connection closed: Socket Handle [%llu]\n", mySocket);
                    return 0;
                } else{
                    (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n\n",
                        msgBuffer,
                        serverAddress.sin_addr.S_un.S_un_b.s_b1,
                        serverAddress.sin_addr.S_un.S_un_b.s_b2,
                        serverAddress.sin_addr.S_un.S_un_b.s_b3,
                        serverAddress.sin_addr.S_un.S_un_b.s_b4,
                        ntohs(serverAddress.sin_port),
                        result
                    );

                    (void)printf("[I/O multiplexing client] Enter msg: ");
                }
            }
        }
    }

    (void)closesocket(mySocket);
    (void)WSACleanup();
}