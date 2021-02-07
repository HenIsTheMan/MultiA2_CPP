#include "Winsock.h"

#include "Pseudorand.h"

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
    const int clientsSize = (int)activeClients.size();

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
            (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes read: %d)\n",
                msgBuffer,
                client0->address.sin_addr.S_un.S_un_b.s_b1,
                client0->address.sin_addr.S_un.S_un_b.s_b2,
                client0->address.sin_addr.S_un.S_un_b.s_b3,
                client0->address.sin_addr.S_un.S_un_b.s_b4,
                ntohs(client0->address.sin_port),
                result
            );

            std::string pureStr(msgBuffer);
            int pureStrLen = pureStr.length();
            char pureDelimiter = '\0';
            std::vector<int> pureDelimiterPos;

            for(int i = 0; i < pureStrLen; ++i) {
                if(pureStr[i] == pureDelimiter) {
                    if(i < pureStrLen - 1 && pureStr[i + 1] == pureDelimiter) {
                        break;
                    }
                    pureDelimiterPos.emplace_back(i);
                }
            }

            int pureDelimiterPosSize = (int)pureDelimiterPos.size();
            std::vector<std::string> rawStrs;

            for(int i = 0; i < pureDelimiterPosSize; ++i) {
                if(i == 0) {
                    rawStrs.emplace_back(pureStr.substr(0, pureDelimiterPos[0]));
                } else {
                    int startIndex = pureDelimiterPos[i - 1] + 1;
                    rawStrs.emplace_back(pureStr.substr(startIndex, pureDelimiterPos[i] - startIndex));
                }
            }

            int rawStrsSize = (int)rawStrs.size();
            for(int i = 0; i < rawStrsSize; ++i) {
                std::string rawStr = rawStrs[i];
                int rawStrLen = rawStr.length();
                char delimiter = ' ';

                std::vector<int> delimiterPos;
                for(int j = 0, count = 0; j < rawStrLen && count <= 2; ++j) {
                    if(rawStr[j] == delimiter) {
                        delimiterPos.emplace_back(j);
                        ++count;
                    }
                }

                std::vector<std::string> txts{
                    rawStr.substr(0, delimiterPos[0]),
                    rawStr.substr(delimiterPos[0] + 1, delimiterPos[1] - (delimiterPos[0] + 1)),
                    rawStr.substr(delimiterPos[1] + 1, rawStrLen - delimiterPos[1] - 1)
                };

                ///
                for(auto str: txts){
                    (void)puts(str.c_str());
                }

                if(txts[1].length() == 1){
                    const std::string normalMsg = "-1 / "
                        + client0->username + delimiter
                        + std::to_string(client0->colorR) + delimiter
                        + std::to_string(client0->colorG) + delimiter
                        + std::to_string(client0->colorB) + delimiter
                        + txts[2];

                    const char* const normalMsgCStr = normalMsg.c_str();
                    
                    for(Client* const client1: activeClients){
                        result = send(client1->mySocket, normalMsgCStr, normalMsg.length(), 0);

                        (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n\n",
                            normalMsgCStr,
                            client1->address.sin_addr.S_un.S_un_b.s_b1,
                            client1->address.sin_addr.S_un.S_un_b.s_b2,
                            client1->address.sin_addr.S_un.S_un_b.s_b3,
                            client1->address.sin_addr.S_un.S_un_b.s_b4,
                            ntohs(client1->address.sin_port),
                            result
                        );
                    }
                } else{
                    const std::string commandIdentifier = txts[1].substr(1);
                    if(commandIdentifier == "NewClientJoined"){
                        client0->username = txts[2];
                        client0->colorR = PseudorandMinMax(0.0f, 1.0f);
                        client0->colorG = PseudorandMinMax(0.0f, 1.0f);
                        client0->colorB = PseudorandMinMax(0.0f, 1.0f);
                    }
                }
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