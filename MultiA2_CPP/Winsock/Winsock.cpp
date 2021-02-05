#include "Winsock.h"

void Winsock::Init(const InitParams& params){
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        return (void)printf("WSAStartup() error!");
    }
}