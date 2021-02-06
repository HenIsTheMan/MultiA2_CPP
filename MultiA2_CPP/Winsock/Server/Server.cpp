#include "Server.h"

#include <fstream>

void Server::Init(const Server::InitParams& params){
    if(params.portNumber != -999){
        portNumber = params.portNumber;
    }

	std::string line;
	std::ifstream stream;
    std::string IPAddress;

	(void)system("ipconfig > ipconfig.txt");
	stream.open("ipconfig.txt"); 

	if(stream.is_open()){
		while(!stream.eof()){
			(void)getline(stream, line);
			if(line.find("IPv4 Address. . . . . . . . . . . :", 0) != std::string::npos){
				line.erase(0,39);
                IPAddress = line;
				stream.close();
			}
		}
	}
    (void)remove("ipconfig.txt");

    (void)printf("[I/O multiplexing server] [%s:%d] Waiting for clients to connect...\n\n", IPAddress.c_str(), portNumber);

    mySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(mySocket == INVALID_SOCKET){
        return (void)printf("socket failed with error %d\n", WSAGetLastError());
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(portNumber);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(mySocket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR){
        return (void)printf("bind failed with error %d\n", WSAGetLastError());
    }

    FD_ZERO(&readFDS);

    //if(listen(mySocket, 5) == SOCKET_ERROR){
    //    return (void)printf("listen failed with error %d\n", WSAGetLastError());
    //}
}