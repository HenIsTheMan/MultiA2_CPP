#include "Server.h"

void Server::Init(const Server::InitParams& params){
    if(params.portNumber != -999){
        portNumber = params.portNumber;
    }

    (void)printf("[I/O multiplexing server] Waiting for clients to connect...\n\n");

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == INVALID_SOCKET){
        return (void)printf("socket failed with error %d\n", WSAGetLastError());
    }
}