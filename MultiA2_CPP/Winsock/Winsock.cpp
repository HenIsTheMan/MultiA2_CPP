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
            FD_SET(server->mySocket, &tempFDS);

            if((result = select(server->mySocket, &tempFDS, 0, 0, &timeout)) == SOCKET_ERROR){
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

                RunChiefProcess(server);
            }
        }
    }
}

void Winsock::RunChiefProcess(Server* const server){
    for(int i = 0; i < (int)tempFDS.fd_count; ++i){
        SOCKET& currSocket = tempFDS.fd_array[i];

        if(currSocket == server->mySocket){
            OnClientConnected(server);
        } else{
            memset(msgBuffer, '\0', msgBufferSize);
            for(Client* const client: activeClients){
                if(client->mySocket == currSocket){
                    int len = sizeof(client->address);
                    result = recvfrom(server->mySocket, msgBuffer, msgBufferSize, 0, (struct sockaddr*)&client->address, &len);
                    break;
                }
            }

            if(result <= 0){
                OnClientDisconnected(server, currSocket);
            } else{
                //ProcessRS(currSocket);
            }
        }
    }
}

void Winsock::OnClientConnected(Server* const server){
    Client* const client = ActivateClient();
    if ((client->mySocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    }

    FD_SET(client->mySocket, &tempFDS);
    //FD_SET(client->mySocket, &server->readFDS);

    #if _WIN64
        (void)printf("Client connected: Socket Handle [%llu]\n\n", client->mySocket);
    #else
        (void)printf("Client connected: Socket Handle [%u]\n\n", client->mySocket);
    #endif
}

void Winsock::OnClientDisconnected(Server* const server, SOCKET& currSocket){
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
}

void Winsock::ProcessRS(SOCKET& currSocket){
    for(Client* const client0: activeClients){
        if(client0->mySocket == currSocket){
            const std::string rawStr(msgBuffer);

            if(rawStr[0] == '~' && rawStr[1] == '/') {
                const int rawStrLen = rawStr.length();

                std::vector<int> spacePosIndices;
                spacePosIndices.reserve(rawStrLen);
                for(int i = 0; i < rawStrLen; ++i){
                    if(rawStr[i] == ' ') {
                        spacePosIndices.emplace_back(i);
                    }
                }

                const int spacePosIndicesSize = (int)spacePosIndices.size();
                if(spacePosIndicesSize > 0){
                    std::vector<std::string> txts;
                    txts.reserve(spacePosIndicesSize);
                    for(int i = 0; i < spacePosIndicesSize; ++i) {
                        if(i == 0) {
                            txts.emplace_back(rawStr.substr(0, spacePosIndices[0]));
                        } else {
                            txts.emplace_back(rawStr.substr(spacePosIndices[i - 1] + 1, spacePosIndices[i] - (spacePosIndices[i - 1] + 1)));

                            if(i == spacePosIndicesSize - 1 && spacePosIndices[i] + 1 < rawStrLen) {
                                txts.emplace_back(rawStr.substr(spacePosIndices[i] + 1, rawStrLen - 1 - spacePosIndices[i]));
                            }
                        }
                    }

                    const std::string commandIdentifier = txts[0].substr(2);
                    if(commandIdentifier == "AddClient") {
                        const int clientIndex = stoi(txts[1]);
                        Client* const client = activeClients[clientIndex];

                        client->index = clientIndex;
                        client->username = txts[2];
                        client->colorR = stof(txts[3]);
                        client->colorG = stof(txts[4]);
                        client->colorB = stof(txts[5]);
                    }
                }
            }

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