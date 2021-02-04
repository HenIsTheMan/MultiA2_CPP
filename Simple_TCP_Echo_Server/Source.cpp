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
    SOCKET serverSocket = INVALID_SOCKET; // Server socket for accepting client
    SOCKET connectedSocket = INVALID_SOCKET; // Connected socket from client
    sockaddr_in serverAddress{};
    sockaddr_in clientAddress{};
    char msgBuffer[bufferLen]{};
    int sizeOfClientAddress = sizeof(clientAddress);
    int result = 0;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != NO_ERROR){
        (void)printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        (void)WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    ///----------------------
    /// 3-2. Bind the socket with server address & port number
    if(SOCKET_ERROR == bind(serverSocket, (SOCKADDR*)&serverAddress,
        sizeof(serverAddress)))
    {
        printf("bind failed with error %u\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    } else
    {
        printf("bind returned success\n");
    }

    ///----------------------
    /// 4. Places a socket in a state in which it is listening for an incoming
    ///    connection.
    if(SOCKET_ERROR == listen(serverSocket, SOMAXCONN))
    {
        printf("listen function failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Waiting for client to connect...\n");
    ///----------------------
    /// 5. Permits an incoming connection attempt on a socket
    connectedSocket = accept(serverSocket, (struct sockaddr*)&clientAddress,
        &sizeOfClientAddress);
    if(INVALID_SOCKET == connectedSocket)
    {
        printf("accept failed with error: %ld\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    } else{
        printf("Client connected. IP Address : %d.%d.%d.%d, Port Number :%d\n",
            //--------------------
            clientAddress.sin_addr.S_un.S_un_b.s_b1,
            clientAddress.sin_addr.S_un.S_un_b.s_b2,
            clientAddress.sin_addr.S_un.S_un_b.s_b3,
            clientAddress.sin_addr.S_un.S_un_b.s_b4,
            ntohs(clientAddress.sin_port));

    }

    ///----------------------
    /// 6. Send & receive the data on a connected socket
    while(1)
    {
        memset(msgBuffer, '\0', bufferLen);
        result = recv(connectedSocket, msgBuffer, bufferLen, 0);
        if(0 < result)
        {
            printf("Bytes received  : %d\n", result);
            printf("Buffer received : %s\n", msgBuffer);
        } else if(0 == result)
        {
            printf("Connection closed\n");
            break;
        } else
        {
            printf("Recv failed: %d\n", WSAGetLastError());
            break;
        }

        /// Echo same message to client
        result = send(connectedSocket, msgBuffer, result, 0);
        if(SOCKET_ERROR == result)
        {
            printf("Send failed: %d\n", WSAGetLastError());
            break;
        }
        printf("Bytes sent : %d\n", result);
    }

    ///----------------------
    /// 7. Closes an existing socket
    closesocket(connectedSocket);
    closesocket(serverSocket);

    ///----------------------
    /// 8. Terminate use of the Winsock Library
    WSACleanup();
    return 0;
}