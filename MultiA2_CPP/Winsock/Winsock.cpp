#include "Winsock.h"

void Winsock::Init(const InitParams& params){
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        return (void)printf("WSAStartup() error!");
    }

    servers.reserve(params.amtOfServers);
    for(int i = 0; i < params.amtOfServers; ++i){
        Server* const server = new Server();
        Server::InitParams serverParams;

        serverParams.portNumber = params.portNumber;

        server->Init(serverParams);
        servers.emplace_back(server);
    }
}