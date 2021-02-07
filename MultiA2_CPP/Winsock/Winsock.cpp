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
            result = recv(currSocket, msgBuffer, msgBufferSize, 0);

            if(result <= 0){
                OnClientDisconnected(server, currSocket);
            } else{
                ProcessRS(currSocket);
            }
        }
    }
}

void Winsock::OnClientConnected(Server* const server){
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

    const std::string updateClientsMsg = "~/UpdateClients";
    const char* const updateClientsMsgCstr = updateClientsMsg.c_str();
    result = send(client->mySocket, updateClientsMsgCstr, updateClientsMsg.length() + 1, 0); //??

    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n\n",
        updateClientsMsgCstr,
        client->address.sin_addr.S_un.S_un_b.s_b1,
        client->address.sin_addr.S_un.S_un_b.s_b2,
        client->address.sin_addr.S_un.S_un_b.s_b3,
        client->address.sin_addr.S_un.S_un_b.s_b4,
        ntohs(client->address.sin_port),
        result
    );
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
                } else{
                    const std::string commandIdentifier = rawStr.substr(2);
                    if(commandIdentifier == "UpdateClients"){
                        //...??
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