#pragma once

#include "Client/Client.hpp"
#include "Server/Server.hpp"

struct Winsock final{
	_6_DEFAULTED(Winsock)

	int result = 0;

	static const int msgBufferSize = 1024;
	char msgBuffer[msgBufferSize]{};

	WSAData wsaData{};
	fd_set readFDS{};
	fd_set tempFDS{};
	TIMEVAL timeout{};

	std::vector<Client*> clients;
	std::vector<Server*> servers;

	//struct InitParams final{
	//	_6_DEFAULTED(InitParams)


	//};

	//void Init(const 

	//if(argc == 2){
	//	portNumber = atoi(argv[1]);
	//}
};