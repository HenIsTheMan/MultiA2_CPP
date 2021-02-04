#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main(const int, const char* const* const){
    const int bufferLen = 1024;
    const int portNumber = 9876;

    WSADATA wsaData{};
    SOCKET mySocket = 0;
    SOCKADDR_IN receiverAddress{};
    SOCKADDR_IN senderAddress{};
    char msgBuffer[bufferLen]{};
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
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    result = bind(mySocket, (SOCKADDR*)&receiverAddress, sizeof(receiverAddress));
    if(result != 0){
        (void)printf("bind failed with error %d\n", WSAGetLastError());
        closesocket(mySocket);
        WSACleanup();
        return 1;
    }

    for(;;){
        printf("[UDP Echo Server] Waiting for datagrams...\n");
        memset(msgBuffer, '\0', bufferLen);

        result = recvfrom(mySocket, msgBuffer, bufferLen, 0,
            (SOCKADDR*)&senderAddress, &sizeOfSenderAddress);
        if( SOCKET_ERROR == result )
        {
            printf("recvfrom failed with error %d\n", WSAGetLastError());
            break;
        }
        else if( 0 == result )
        {
            printf("Connection closed");
            break;
        }

        printf("Read bytes = %d, Message = [%s], from %d.%d.%d.%d:%d\n",
            result, msgBuffer,
            senderAddress.sin_addr.S_un.S_un_b.s_b1,
            senderAddress.sin_addr.S_un.S_un_b.s_b2,
            senderAddress.sin_addr.S_un.S_un_b.s_b3,
            senderAddress.sin_addr.S_un.S_un_b.s_b4,
            ntohs(senderAddress.sin_port));

        ///---------------------------------------------
        /// Send a datagram to the receiver
        printf("Sending a datagram to the receiver...\n");
        result = sendto( mySocket, msgBuffer, result, 0,
            (SOCKADDR *)&senderAddress, sizeof(senderAddress) );
        if( SOCKET_ERROR == result )
        {
            wprintf(L"sendto failed with error %d\n", WSAGetLastError());
            break;
        }
    }

    ///-----------------------------------------------
    /// 5. Close the socket when finished receiving datagrams
    if(SOCKET_ERROR == result)
    {
        closesocket( mySocket );
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Finished receiving. Closing socket.\n");
        result = closesocket( mySocket );
        if( SOCKET_ERROR == result )
        {
            printf("closesocket failed with error %d\n", WSAGetLastError());
            return 1;
        }
    }

    ///-----------------------------------------------
    /// Clean up and exit.
    printf("Exiting.\n");
    WSACleanup();
    return 0;
}