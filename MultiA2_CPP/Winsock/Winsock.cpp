#include "Winsock.h"

Winsock::~Winsock(){
    if(clientPool != nullptr){
        clientPool->Destroy();
        clientPool = nullptr;
    }

    if(serverPool != nullptr){
        serverPool->Destroy();
        serverPool = nullptr;
    }

    (void)WSACleanup();
}

void Winsock::Init(const InitParams& params){
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        return (void)printf("WSAStartup() error!");
    }

    clientPool->Init(params.clientPoolInactiveSize, params.clientPoolActiveSize);
    serverPool->Init(params.serverPoolInactiveSize, params.serverPoolActiveSize);

    FD_ZERO(&readFDS);
}

void Winsock::Run(){
    static int timeoutCounter = 0;

    for(;;){
        tempFDS = readFDS;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        if((result = select(0, &tempFDS, 0, 0, &timeout)) == SOCKET_ERROR){
            (void)printf("select() error\n");
            return 1;
        }

        if(result == 0){
            if(timeoutCounter > 0){
                (void)printf("\033[A\033[A\33[2K");
            }
            (void)printf("select() returned by timeout (%d)\n\n", ++timeoutCounter);
        } else if(result < 0){
            timeoutCounter = 0;
            (void)printf("select() error\n");
        } else{
            timeoutCounter = 0;

            for(int i = 0; i < (int)tempFDS.fd_count; ++i){
                SOCKET& currSocket = tempFDS.fd_array[i];

                if(currSocket == serverSocket){
                    clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &sizeOfClientAddress);
                    if(clientSocket == INVALID_SOCKET){
                        (void)printf("accept failed with error %ld\n", WSAGetLastError());
                        (void)closesocket(serverSocket);
                        (void)WSACleanup();
                        return 1;
                    }

                    FD_SET(clientSocket, &readFDS);

                    #if _WIN64
                    (void)printf("Client connected: Socket Handle [%llu]\n\n", clientSocket);
                    #else
                    (void)printf("Client connected: Socket Handle [%u]\n\n", clientSocket);
                    #endif
                } else{
                    memset(msgBuffer, '\0', bufferSize);
                    result = recv(currSocket, msgBuffer, bufferSize, 0);

                    if(result <= 0){
                        #if _WIN64
                        (void)printf("Client disconnected: Socket Handle [%llu]\n\n", currSocket);
                        #else
                        (void)printf("Client disconnected: Socket Handle [%u]\n\n", currSocket);
                        #endif

                        (void)closesocket(currSocket);

                        FD_CLR(currSocket, &readFDS);
                    } else{
                        (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n",
                            msgBuffer,
                            clientAddress.sin_addr.S_un.S_un_b.s_b1,
                            clientAddress.sin_addr.S_un.S_un_b.s_b2,
                            clientAddress.sin_addr.S_un.S_un_b.s_b3,
                            clientAddress.sin_addr.S_un.S_un_b.s_b4,
                            ntohs(clientAddress.sin_port),
                            result
                        );

                        result = send(currSocket, msgBuffer, result, 0);

                        (void)printf("\"%s\" (to %d.%d.%d.%d: %d, bytes sent: %d)\n\n",
                            msgBuffer,
                            clientAddress.sin_addr.S_un.S_un_b.s_b1,
                            clientAddress.sin_addr.S_un.S_un_b.s_b2,
                            clientAddress.sin_addr.S_un.S_un_b.s_b3,
                            clientAddress.sin_addr.S_un.S_un_b.s_b4,
                            ntohs(clientAddress.sin_port),
                            result
                        );
                    }
                }
            }
        }
    }
}

void Winsock::ActivateClient() const{
    clientPool->ActivateObj();
}

void Winsock::ActivateServer() const{
    Server* const server = serverPool->ActivateObj();

    FD_SET(server->mySocket, &readFDS);
}

void Winsock::DeactivateClient(Client* const client) const{
    clientPool->DeactivateObj(client);
}

void Winsock::DeactivateServer(Server* const server) const{
    serverPool->DeactivateObj(server);
}