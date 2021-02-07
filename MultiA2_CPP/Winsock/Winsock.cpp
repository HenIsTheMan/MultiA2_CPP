#include "Winsock.h"

#include "Color.hpp"
#include "Pseudorand.hpp"

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

            const std::string rawStr(msgBuffer);
            const int rawStrLen = rawStr.length();
            const char delimiter = ' ';

            std::vector<int> delimiterPos;
            for(int j = 0, count = 0; j < rawStrLen && count < 2; ++j) {
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

            if(txts[1].length() == 1){
                const std::string normalMsg = txts[0] + " / "
                    + client0->username + delimiter
                    + std::to_string(client0->colorR) + delimiter
                    + std::to_string(client0->colorG) + delimiter
                    + std::to_string(client0->colorB) + delimiter
                    + txts[2];

                const char* const normalMsgCStr = normalMsg.c_str();
                    
                for(Client* const client1: activeClients){
                    if(txts[0] == "0" || (txts[0] == "1" && client1 == client0)){
                        result = send(client1->mySocket, normalMsgCStr, normalMsg.length() + 1, 0); //+1 for a '\0'

                        (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                            normalMsgCStr,
                            client1->address.sin_addr.S_un.S_un_b.s_b1,
                            client1->address.sin_addr.S_un.S_un_b.s_b2,
                            client1->address.sin_addr.S_un.S_un_b.s_b3,
                            client1->address.sin_addr.S_un.S_un_b.s_b4,
                            ntohs(client1->address.sin_port),
                            result
                        );
                    }
                }
            } else{
                std::string commandIdentifier = txts[1].substr(1);

                std::for_each(commandIdentifier.begin(), commandIdentifier.end(), [](char & c){
                    c = ::tolower(c);
                });

                if(commandIdentifier == "newclientjoined"){
                    client0->username = txts[2];

                    const Color randColorRGB = Color::HSVToRGB({PseudorandMinMax(0.0f, 360.0f), PseudorandMinMax(0.0f, 1.0f), PseudorandMinMax(0.0f, 0.7f)});
                    client0->colorR = randColorRGB.r;
                    client0->colorG = randColorRGB.g;
                    client0->colorB = randColorRGB.b;

                    client0->isAfk = false;

                    static const std::string otherWelcomeMsgPostfixes[]{
                        " just joined the server!",
                        " just hopped in!",
                        " has just arrived!",
                        " has entered the chat!",
                    };
                    static const int otherWelcomeMsgPostfixesSize = int(sizeof(otherWelcomeMsgPostfixes) / sizeof(otherWelcomeMsgPostfixes[0]));

                    const std::string welcomeMsg = "1 / Server 0.0 0.0 0.0 Welcome to Virtual Chat!";
                    const char* const welcomeMsgCStr = welcomeMsg.c_str();
                    const std::string otherWelcomeMsg = "1 / Server 0.0 0.0 0.0 " + client0->username + otherWelcomeMsgPostfixes[PseudorandMinMax(0, otherWelcomeMsgPostfixesSize - 1)];
                    const char* const otherWelcomeMsgCStr = otherWelcomeMsg.c_str();

                    for(Client* const client1: activeClients){
                        if(client1 == client0){
                            result = send(client1->mySocket, welcomeMsgCStr, welcomeMsg.length() + 1, 0); //+1 for a '\0'

                            (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                                welcomeMsgCStr,
                                client1->address.sin_addr.S_un.S_un_b.s_b1,
                                client1->address.sin_addr.S_un.S_un_b.s_b2,
                                client1->address.sin_addr.S_un.S_un_b.s_b3,
                                client1->address.sin_addr.S_un.S_un_b.s_b4,
                                ntohs(client1->address.sin_port),
                                result
                            );
                        } else if(client1->username != ""){
                            result = send(client1->mySocket, otherWelcomeMsgCStr, otherWelcomeMsg.length() + 1, 0); //+1 for a '\0'

                            (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                                otherWelcomeMsgCStr,
                                client1->address.sin_addr.S_un.S_un_b.s_b1,
                                client1->address.sin_addr.S_un.S_un_b.s_b2,
                                client1->address.sin_addr.S_un.S_un_b.s_b3,
                                client1->address.sin_addr.S_un.S_un_b.s_b4,
                                ntohs(client1->address.sin_port),
                                result
                            );
                        }
                    }
                } else if(commandIdentifier == "count"){
                    const std::string countMsg = "1 / Server 0.0 0.0 0.0 Number of users online --- " + std::to_string(activeClients.size());
                    const char* const countMsgCStr = countMsg.c_str();

                    result = send(client0->mySocket, countMsgCStr, countMsg.length() + 1, 0); //+1 for a '\0'

                    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                        countMsgCStr,
                        client0->address.sin_addr.S_un.S_un_b.s_b1,
                        client0->address.sin_addr.S_un.S_un_b.s_b2,
                        client0->address.sin_addr.S_un.S_un_b.s_b3,
                        client0->address.sin_addr.S_un.S_un_b.s_b4,
                        ntohs(client0->address.sin_port),
                        result
                    );
                } else if(commandIdentifier == "clear"){
                    result = send(client0->mySocket, msgBuffer, msgBufferSize, 0);

                    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                        msgBuffer,
                        client0->address.sin_addr.S_un.S_un_b.s_b1,
                        client0->address.sin_addr.S_un.S_un_b.s_b2,
                        client0->address.sin_addr.S_un.S_un_b.s_b3,
                        client0->address.sin_addr.S_un.S_un_b.s_b4,
                        ntohs(client0->address.sin_port),
                        result
                    );
                } else if(commandIdentifier == "wipe"){
                    for(Client* const client1: activeClients){
                        result = send(client1->mySocket, msgBuffer, msgBufferSize, 0);

                        (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                            msgBuffer,
                            client1->address.sin_addr.S_un.S_un_b.s_b1,
                            client1->address.sin_addr.S_un.S_un_b.s_b2,
                            client1->address.sin_addr.S_un.S_un_b.s_b3,
                            client1->address.sin_addr.S_un.S_un_b.s_b4,
                            ntohs(client1->address.sin_port),
                            result
                        );
                    }
                } else if(commandIdentifier == "me"){
                } else if(commandIdentifier == "afk"){
                    client0->isAfk = !client0->isAfk;

                    const std::string afkMsg = "1 / Server 0.0 0.0 0.0 You are now " + std::string(client0->isAfk ? "" : "not ") + "afk...";
                    const char* const afkMsgCStr = afkMsg.c_str();

                    result = send(client0->mySocket, afkMsgCStr, afkMsg.length() + 1, 0); //+1 for a '\0'

                    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                        afkMsgCStr,
                        client0->address.sin_addr.S_un.S_un_b.s_b1,
                        client0->address.sin_addr.S_un.S_un_b.s_b2,
                        client0->address.sin_addr.S_un.S_un_b.s_b3,
                        client0->address.sin_addr.S_un.S_un_b.s_b4,
                        ntohs(client0->address.sin_port),
                        result
                    );
                } else{
                    const std::string unknownCommandMsg = "1 / Server 0.0 0.0 0.0 Unrecognized command!" + (std::string)" \"" + txts[1] + '\"';
                    const char* const unknownCommandMsgCStr = unknownCommandMsg.c_str();

                    result = send(client0->mySocket, unknownCommandMsgCStr, unknownCommandMsg.length() + 1, 0); //+1 for a '\0'

                    (void)printf("\"%s\" [%d.%d.%d.%d:%d] (bytes sent: %d)\n",
                        unknownCommandMsgCStr,
                        client0->address.sin_addr.S_un.S_un_b.s_b1,
                        client0->address.sin_addr.S_un.S_un_b.s_b2,
                        client0->address.sin_addr.S_un.S_un_b.s_b3,
                        client0->address.sin_addr.S_un.S_un_b.s_b4,
                        ntohs(client0->address.sin_port),
                        result
                    );
                }
            }

            (void)printf("\n");
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