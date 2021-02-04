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
    const int defaultBufferLen = 1024;
    const char* const serverIPAddress = "127.0.0.1";
    const int portNumber = 9876;

    WSADATA wsaData;
    SOCKET receiverSocket; //For receiving datagrams
    SOCKADDR_IN receiverAddress{};
    SOCKADDR_IN senderAddress{};
    char msgBuffer[defaultBufferLen]{};
    int msgSize = 0;
    int result = 0;
    int sizeOfSenderAddress = sizeof(senderAddress);

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != NO_ERROR){ //Init Winsock
        (void)printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    receiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(receiverSocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    ///Set up the receiverAddress structure with the IP address of the receiver (in this example case "192.168.1.1") ??
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(portNumber);
    receiverAddress.sin_addr.s_addr = inet_addr(serverIPAddress);

    for(;;){
        (void)printf("Enter msgs: ");

        int i = 0;
        while(i < defaultBufferLen - 1){
            msgBuffer[i] = getchar();
            if(msgBuffer[i] == '\n'){
                msgBuffer[i++] = '\0';
                break;
            }
            ++i;
        }
        msgSize = i;

        result = sendto(receiverSocket, msgBuffer, msgSize, 0, (SOCKADDR*)&receiverAddress, sizeof(receiverAddress)); //Send a datagram
        if(result == SOCKET_ERROR){
            (void)printf("sendto failed with error: %d\n", WSAGetLastError());
            break;
        }

        result = recvfrom(receiverSocket, msgBuffer, defaultBufferLen, 0, (SOCKADDR*)&senderAddress, &sizeOfSenderAddress); //Receive datagrams
        if(result == SOCKET_ERROR){
            (void)printf("recvfrom failed with error %d\n", WSAGetLastError());
            break;
        } else if(result == 0){
            (void)printf("Socket closed");
            break;
        }

        printf("Read bytes = %d, Message = [%s], from %d.%d.%d.%d:%d\n",
            result,
            msgBuffer,
            senderAddress.sin_addr.S_un.S_un_b.s_b1,
            senderAddress.sin_addr.S_un.S_un_b.s_b2,
            senderAddress.sin_addr.S_un.S_un_b.s_b3,
            senderAddress.sin_addr.S_un.S_un_b.s_b4,
            ntohs(senderAddress.sin_port)
        );
    }

    result = closesocket(receiverSocket);
    if(result == SOCKET_ERROR){
        printf("closesocket failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    WSACleanup();
}