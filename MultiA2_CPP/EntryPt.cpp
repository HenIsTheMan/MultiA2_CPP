#include "Winsock/Winsock.h"

int main(const int argc, const char* const* const argv){
    Winsock* winsock = new Winsock();
    Winsock::InitParams params;

    params.clientPoolInactiveSize = 999;
    params.clientPoolActiveSize = 999;
    params.serverPoolInactiveSize = 7;
    params.serverPoolActiveSize = 7;

    winsock->Init(params);

    Server* const server = winsock->ActivateServer();
    if(argc == 2){
        server->Init({
            atoi(argv[1])
        });
    } else{
        server->Init({
            7890
        });
    }
    FD_SET(server->mySocket, &winsock->readFDS);

    winsock->Run();

    if(winsock != nullptr){
        delete winsock;
        winsock = nullptr;
    }
}