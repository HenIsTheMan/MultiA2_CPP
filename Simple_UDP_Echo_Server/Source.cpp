//------------------------------
// Lab01. Exercise5
// Simple UDP/IP Echo Server
// Filename : simple_udp_echo_server.cpp

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdio.h>

/// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define PORT_NUMBER    9876

int main(void)
{
    WSADATA     wsaData;
    SOCKET      RecvSocket;
    SOCKADDR_IN RecvAddr;
    SOCKADDR_IN SenderAddr;
    int         SenderAddrSize;
    char        RecvBuf[DEFAULT_BUFLEN];
    int         MessageDataLen;
    int         iResult;

    ///-----------------------------------------------
    /// 1. Initialize Winsock
    iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if( NO_ERROR != iResult )
    {
        printf("WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    ///-----------------------------------------------
    /// 2. Create a receiver socket to receive datagrams
    RecvSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( INVALID_SOCKET == RecvSocket )
    {
        printf("socket failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    ///-----------------------------------------------
    /// 3. Bind the socket to any address and the specified port.
    RecvAddr.sin_family      = AF_INET;
    RecvAddr.sin_port        = htons(PORT_NUMBER);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind( RecvSocket, (SOCKADDR *)& RecvAddr, sizeof (RecvAddr) );
    if( 0 != iResult )
    {
        printf("bind failed with error %d\n", WSAGetLastError());
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    while(1)
    {
        ///-----------------------------------------------
        /// 4. Call the recvfrom function to receive datagrams on the bound socket.
        printf("Receiving datagrams...\n");
        memset(RecvBuf, '\0', DEFAULT_BUFLEN);
        SenderAddrSize = sizeof(SenderAddr);
        iResult = recvfrom( RecvSocket, RecvBuf, DEFAULT_BUFLEN, 0,
            (SOCKADDR *)&SenderAddr, &SenderAddrSize );
        if( SOCKET_ERROR == iResult )
        {
            printf("recvfrom failed with error %d\n", WSAGetLastError());
            break;
        }
        else if( 0 == iResult )
        {
            printf("Connection closed");
            break;
        }

        printf("Read bytes = %d, Message = [%s], from %d.%d.%d.%d:%d\n",
            iResult, RecvBuf,
            SenderAddr.sin_addr.S_un.S_un_b.s_b1,
            SenderAddr.sin_addr.S_un.S_un_b.s_b2,
            SenderAddr.sin_addr.S_un.S_un_b.s_b3,
            SenderAddr.sin_addr.S_un.S_un_b.s_b4,
            ntohs(SenderAddr.sin_port));

        MessageDataLen = iResult;

        ///---------------------------------------------
        /// Send a datagram to the receiver
        printf("Sending a datagram to the receiver...\n");
        iResult = sendto( RecvSocket, RecvBuf, MessageDataLen, 0,
            (SOCKADDR *)&SenderAddr, sizeof(SenderAddr) );
        if( SOCKET_ERROR == iResult )
        {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    ///-----------------------------------------------
    /// 5. Close the socket when finished receiving datagrams
    if(SOCKET_ERROR == iResult)
    {
        closesocket( RecvSocket );
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Finished receiving. Closing socket.\n");
        iResult = closesocket( RecvSocket );
        if( SOCKET_ERROR == iResult )
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