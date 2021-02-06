#include "Winsock/Winsock.h"

int main(const int argc, const char* const* const argv){
    Winsock* winsock = new Winsock();
    Winsock::InitParams params;

    if(argc == 2){
        params.portNumber = atoi(argv[1]);
    }
    params.clientPoolInactiveSize = 999;
    params.clientPoolActiveSize = 999;
    params.serverPoolInactiveSize = 7;
    params.serverPoolActiveSize = 7;

    winsock->Init(params);
    winsock->ActivateServer();
    winsock->Run();

    if(winsock != nullptr){
        delete winsock;
        winsock = nullptr;
    }
}