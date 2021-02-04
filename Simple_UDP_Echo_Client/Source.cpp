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
    const int bufferSize = 1024;
    const char* const serverIPAddress = "127.0.0.1";
    const int portNumber = 9876;

    WSADATA wsaData{};
    SOCKET mySocket = 0;
    SOCKADDR_IN receiverAddress{};
    SOCKADDR_IN senderAddress{};
    char msgBuffer[bufferSize]{};
    int msgSize = 0;
    int result = 0;
    int sizeOfSenderAddress = sizeof(senderAddress);

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != NO_ERROR){
        (void)printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(mySocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(portNumber);
    receiverAddress.sin_addr.s_addr = inet_addr(serverIPAddress);

    for(;;){
        (void)printf("[UDP Echo Client] Enter msg: ");

        int i = 0;
        while(i < bufferSize - 1){
            msgBuffer[i] = getchar();
            if(msgBuffer[i] == '\n'){
                msgBuffer[i++] = '\0';
                break;
            }
            ++i;
        }
        msgSize = i;

        result = sendto(mySocket, msgBuffer, msgSize, 0, (SOCKADDR*)&receiverAddress, sizeof(receiverAddress)); //Send a datagram
        if(result == SOCKET_ERROR){
            (void)printf("sendto failed with error %d\n", WSAGetLastError());
            break;
        }

        result = recvfrom(mySocket, msgBuffer, bufferSize, 0, (SOCKADDR*)&senderAddress, &sizeOfSenderAddress); //Receive a datagram
        if(result == SOCKET_ERROR){
            (void)printf("recvfrom failed with error %d\n", WSAGetLastError());
            break;
        } else if(result == 0){
            (void)printf("Socket closed");
            break;
        }

        (void)printf("\"%s\" (to %d.%d.%d.%d: %d, bytes read: %d)\n\n",
            msgBuffer,
            senderAddress.sin_addr.S_un.S_un_b.s_b1,
            senderAddress.sin_addr.S_un.S_un_b.s_b2,
            senderAddress.sin_addr.S_un.S_un_b.s_b3,
            senderAddress.sin_addr.S_un.S_un_b.s_b4,
            ntohs(senderAddress.sin_port),
            result
        );
    }

    result = closesocket(mySocket);
    if(result == SOCKET_ERROR){
        (void)printf("closesocket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    (void)WSACleanup();
}