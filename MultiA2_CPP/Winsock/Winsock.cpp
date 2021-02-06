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

    timeout.tv_sec = params.timeoutSec;
    timeout.tv_usec = params.timeoutMicrosec;
}

void Winsock::Run(){
    //static int timeoutCounter = 0;

    for(;;){
        for(Server* const server: activeServers){
            tempFDS = server->readFDS;

            if((result = select(0, &tempFDS, 0, 0, &timeout)) == SOCKET_ERROR){
                return (void)printf("select() error\n");
            }

            if(result == 0){
                //if(timeoutCounter > 0){
                //    (void)printf("\033[A\033[A\33[2K");
                //}

                //(void)printf("select() returned by timeout (%d)\n\n", ++timeoutCounter);
            } else if(result < 0){
                //timeoutCounter = 0;

                (void)printf("select() error\n");
            } else{
                //timeoutCounter = 0;

                for(int i = 0; i < (int)tempFDS.fd_count; ++i){
                    SOCKET& currSocket = tempFDS.fd_array[i];

                    if(currSocket == server->mySocket){
                        Client* const client = ActivateClient();
                        client->mySocket = accept(server->mySocket, (SOCKADDR*)&client->address, &client->sizeOfAddress);

                        if(client->mySocket == INVALID_SOCKET){
                            (void)printf("accept failed with error %ld\n", WSAGetLastError());
                            (void)closesocket(server->mySocket);
                            return (void)WSACleanup();
                        }

                        FD_SET(client->mySocket, &server->readFDS);

                        #if _WIN64
                            (void)printf("Client connected: Socket Handle [%llu]\n\n", client->mySocket);
                        #else
                            (void)printf("Client connected: Socket Handle [%u]\n\n", client->mySocket);
                        #endif
                    } else{
                        memset(msgBuffer, '\0', msgBufferSize);
                        result = recv(currSocket, msgBuffer, msgBufferSize, 0);

                        if(result <= 0){
                            #if _WIN64
                                (void)printf("Client disconnected: Socket Handle [%llu]\n\n", currSocket);
                            #else
                                (void)printf("Client disconnected: Socket Handle [%u]\n\n", currSocket);
                            #endif

                            for(Client* const client: activeClients){
                                if(client->mySocket == currSocket){
                                    DeactivateClient(client);
                                    break;
                                }
                            }

                            (void)closesocket(currSocket);

                            FD_CLR(currSocket, &server->readFDS);
                        } else{
                            for(Client* const client0: activeClients){
                                if(client0->mySocket == currSocket){
                                    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes read: %d)\n",
                                        msgBuffer,
                                        client0->address.sin_addr.S_un.S_un_b.s_b1,
                                        client0->address.sin_addr.S_un.S_un_b.s_b2,
                                        client0->address.sin_addr.S_un.S_un_b.s_b3,
                                        client0->address.sin_addr.S_un.S_un_b.s_b4,
                                        ntohs(client0->address.sin_port),
                                        result
                                    );

                                    for(Client* const client1: activeClients){
                                        result = send(client1->mySocket, msgBuffer, result, 0);

                                        (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n\n",
                                            msgBuffer,
                                            client1->address.sin_addr.S_un.S_un_b.s_b1,
                                            client1->address.sin_addr.S_un.S_un_b.s_b2,
                                            client1->address.sin_addr.S_un.S_un_b.s_b3,
                                            client1->address.sin_addr.S_un.S_un_b.s_b4,
                                            ntohs(client1->address.sin_port),
                                            result
                                        );
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

Client* Winsock::ActivateClient() const{
    return clientPool->ActivateObj();
}

Server* Winsock::ActivateServer() const{
    return serverPool->ActivateObj();
}

void Winsock::DeactivateClient(Client* const client) const{
    clientPool->DeactivateObj(client);
}

void Winsock::DeactivateServer(Server* const server) const{
    serverPool->DeactivateObj(server);
}