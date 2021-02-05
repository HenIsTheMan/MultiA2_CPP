#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

int main(const int argc, const char* const* const argv){
    const int bufferSize = 1024;
    int portNumber = 6969;
    WSAData wsaData{};
    SOCKET clientSocket = 0;
    SOCKET serverSocket = 0;
    SOCKADDR_IN clientAddress{};
    SOCKADDR_IN serverAddress{};
    int sizeOfClientAddress = sizeof(SOCKADDR_IN);
    fd_set readFDS{};
    fd_set tempFDS{};
    TIMEVAL timeout{};
    char msgBuffer[bufferSize]{};
    int result = 0;

    if(argc == 2){
        portNumber = atoi(argv[1]);
    }
    printf("Using port number : [%d]\n", portNumber); //??

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        (void)printf("WSAStartup() error!");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){
        (void)printf("bind failed with error %d\n", WSAGetLastError());
        return 1;
    }

    if(listen(serverSocket, 5) == SOCKET_ERROR){
        (void)printf("listen failed with error %d\n", WSAGetLastError());
        return 1;
    }

    FD_ZERO(&readFDS);
    FD_SET(serverSocket, &readFDS);

    for(;;){
        tempFDS = readFDS;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        if((result = select(0, &tempFDS, 0, 0, &timeout)) == SOCKET_ERROR){
            (void)printf("select() error\n");
            return 1;
        }

        if(result == 0){
            (void)printf("select() returned by timeout.\n");
        } else if(result < 0){
            (void)printf("select() error\n");
        } else{
            for(int i = 0; i < tempFDS.fd_count; ++i){
                if(tempFDS.fd_array[i] == serverSocket){
                    clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &sizeOfClientAddress);
                    if(clientSocket == INVALID_SOCKET){
                        (void)printf("accept failed with error %ld\n", WSAGetLastError());
                        (void)closesocket(serverSocket);
                        (void)WSACleanup();
                        return 1;
                    }

                    FD_SET(clientSocket, &readFDS);
                    (void)printf("New Client Accepted: Socket Handle [%llu]\n", clientSocket); //??
                } else{
                    result = recv(tempFDS.fd_array[i], msgBuffer, bufferSize, 0);
                    if(result == 0){ //Connection closed, msg has arrived
                        (void)closesocket(tempFDS.fd_array[i]);
                        (void)printf("Connection closed: Socket Handle [%llu]\n", tempFDS.fd_array[i]);
                        FD_CLR(tempFDS.fd_array[i], &readFDS);
                    } else if(result < 0){
                        (void)closesocket(tempFDS.fd_array[i]);
                        (void)printf("Err: Socket Handle [%llu]\n", tempFDS.fd_array[i]);
                        FD_CLR(tempFDS.fd_array[i], &readFDS);
                    } else{
                        result = send(tempFDS.fd_array[i], msgBuffer, result, 0);
                    }
                }
            }
        }
    }

    (void)WSACleanup();
}