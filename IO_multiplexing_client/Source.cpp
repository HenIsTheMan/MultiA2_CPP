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

    fd_set       ReadFds, TempFds;
    TIMEVAL      Timeout; // struct timeval timeout;

    char         Message[bufferSize];
    int          MessageLen;
    int          Return;

    printf("Destination IP Address [%s], portNumber number [%d]\n", IPAddress, portNumber);

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        (void)printf("WSAStartup() error!");
        return 1;
    }

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(INVALID_SOCKET == mySocket)
    {
        printf("socket() error");
        return 1;
    }

    ///----------------------
    /// The sockaddr_in structure specifies the address family,
    /// IP address, and portNumber of the server to be connected to.
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(portNumber);
    serverAddress.sin_addr.s_addr = inet_addr(IPAddress);

    ///----------------------
    /// Connect to server.
    Return = connect(mySocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if(Return == SOCKET_ERROR)
    {
        closesocket(mySocket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    FD_ZERO(&ReadFds);
    FD_SET(mySocket, &ReadFds);


    printf("enter messages : ");
    memset(Message, '\0', bufferSize);
    MessageLen = 0;
    while(1)
    {
        if(_kbhit())
        { // To check keyboard input.
            Message[MessageLen] = _getch();
            if(('\n' == Message[MessageLen]) || ('\r' == Message[MessageLen]))
            { // Send the message to server.
                putchar('\n');
                MessageLen++;
                Message[MessageLen] = '\0';

                Return = send(mySocket, Message, MessageLen, 0);
                if(Return == SOCKET_ERROR)
                {
                    printf("send failed: %d\n", WSAGetLastError());
                    closesocket(mySocket);
                    WSACleanup();
                    return 1;
                }
                printf("Bytes Sent: %ld\n", Return);

                MessageLen = 0;
                memset(Message, '\0', bufferSize);
            }
            else
            {
                putchar(Message[MessageLen]);
                MessageLen++;
            }
        }
        else
        {
            TempFds = ReadFds;
            Timeout.tv_sec = 0;
            Timeout.tv_usec = 1000;

            if(SOCKET_ERROR == (Return = select(0, &TempFds, 0, 0, &Timeout)))
            { // Select() function returned error.
                closesocket(mySocket);
                printf("select() error\n");
                return 1;
            }
            else if(0 > Return)
            {
                printf("Select returned error!\n");
            }
            else if(0 < Return)
            {
                memset(Message, '\0', bufferSize);
                printf("Select Processed... Something to read\n");
                Return = recv(mySocket, Message, bufferSize, 0);
                if(0 > Return)
                { // recv() function returned error.
                    closesocket(mySocket);
                    printf("Exceptional error :Socket Handle [%llu]\n", mySocket);
                    return 1;
                }
                else if(0 == Return)
                { // Connection closed message has arrived.
                    closesocket(mySocket);
                    printf("Connection closed :Socket Handle [%llu]\n", mySocket);
                    return 0;
                }
                else
                { // Message received.
                    printf("Bytes received   : %d\n", Return);
                    printf("Message received : %s\n", Message);
                    printf("enter messages : ");
                }
            }
        }
    }

    closesocket(mySocket);
    WSACleanup();
    return 0;
}