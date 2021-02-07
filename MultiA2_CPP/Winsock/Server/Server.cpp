#include "Server.h"

#include <fstream>

void Server::Init(const Server::InitParams& params){
    portNumber = params.portNumber;
    type = params.type;

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

    if(type == ProtocolType::TCP){
        mySocket = socket(AF_INET, SOCK_STREAM, 0);
    } else{
        mySocket = socket(AF_INET, SOCK_DGRAM, 0);
    }
    if(mySocket == INVALID_SOCKET){
        return (void)printf("socket failed with error %d\n", WSAGetLastError());
    }

    if(type == ProtocolType::TCP){
        FD_ZERO(&readFDS);
        FD_SET(mySocket, &readFDS);

        address.sin_family = AF_INET;
        address.sin_port = htons(portNumber);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(mySocket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR){
            return (void)printf("bind failed with error %d\n", WSAGetLastError());
        }

        if(listen(mySocket, 5) == SOCKET_ERROR){
            return (void)printf("listen failed with error %d\n", WSAGetLastError());
        }
    } else{
        address.sin_family = AF_INET;
        address.sin_port = htons(portNumber);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(mySocket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR){
            return (void)printf("bind failed with error %d\n", WSAGetLastError());
        }

        FD_ZERO(&readFDS);
    }
}