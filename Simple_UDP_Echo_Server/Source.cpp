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

    result = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if( NO_ERROR != result )
    {
        printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    ///-----------------------------------------------
    /// 2. Create a receiver socket to receive datagrams
    mySocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( INVALID_SOCKET == mySocket )
    {
        printf("socket failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    ///-----------------------------------------------
    /// 3. Bind the socket to any address and the specified port.
    receiverAddress.sin_family      = AF_INET;
    receiverAddress.sin_port        = htons(portNumber);
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    result = bind( mySocket, (SOCKADDR *)& receiverAddress, sizeof (receiverAddress) );
    if( 0 != result )
    {
        printf("bind failed with error %d\n", WSAGetLastError());
        closesocket(mySocket);
        WSACleanup();
        return 1;
    }

    while(1)
    {
        ///-----------------------------------------------
        /// 4. Call the recvfrom function to receive datagrams on the bound socket.
        printf("Receiving datagrams...\n");
        memset(msgBuffer, '\0', bufferLen);

        result = recvfrom( mySocket, msgBuffer, bufferLen, 0,
            (SOCKADDR *)&senderAddress, &sizeOfSenderAddress );
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
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
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