#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main(const int, const char* const* const){
    const int bufferLen = 1024;
    const int portNumber = 9876;

    WSADATA wsaData{};
    SOCKET clientSocket = 0;
    sockaddr_in serverAddress{};
    char msgBuffer[bufferLen]{};
    int msgSize = 0;
    int result = 0;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != NO_ERROR){
        (void)printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        (void)WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //127.0.0.1 is loopback address

    if(connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){
        (void)closesocket(clientSocket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        (void)WSACleanup();
        return 1;
    }
    printf("Connected to server...\n\n");

    //* For welcome msg
    memset(msgBuffer, '\0', bufferLen);
    result = recv(clientSocket, msgBuffer, bufferLen, 0);
    if(result > 0){
        (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n\n",
            msgBuffer,
            serverAddress.sin_addr.S_un.S_un_b.s_b1,
            serverAddress.sin_addr.S_un.S_un_b.s_b2,
            serverAddress.sin_addr.S_un.S_un_b.s_b3,
            serverAddress.sin_addr.S_un.S_un_b.s_b4,
            ntohs(serverAddress.sin_port),
            result
        );
    } else{
        result == 0 ? (void)printf("Connection closed\n") : (void)printf("recv failed with error %d\n", WSAGetLastError());
    }
    //*/

    for(;;){
        (void)printf("[TCP Echo Client] Enter msg: ");

        int i = 0;
        while(i < bufferLen - 1){
            msgBuffer[i] = getchar();
            if(msgBuffer[i] == '\n'){
                msgBuffer[i++] = '\0';
                break;
            }
            ++i;
        }
        msgSize = i;

        if((result = send(clientSocket, msgBuffer, msgSize, 0)) == SOCKET_ERROR){
            (void)printf("send failed with error %d\n", WSAGetLastError());
            (void)closesocket(clientSocket);
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

        memset(msgBuffer, '\0', bufferLen);
        result = recv(clientSocket, msgBuffer, bufferLen, 0);
        if(result > 0){
            (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n\n",
                msgBuffer,
                serverAddress.sin_addr.S_un.S_un_b.s_b1,
                serverAddress.sin_addr.S_un.S_un_b.s_b2,
                serverAddress.sin_addr.S_un.S_un_b.s_b3,
                serverAddress.sin_addr.S_un.S_un_b.s_b4,
                ntohs(serverAddress.sin_port),
                result
            );
        } else{
            result == 0 ? (void)printf("Connection closed\n") : (void)printf("recv failed with error %d\n", WSAGetLastError());
        }
    }

    (void)closesocket(clientSocket);
    (void)WSACleanup();
}