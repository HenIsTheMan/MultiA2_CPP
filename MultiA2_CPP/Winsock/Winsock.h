#pragma once

#include "Client/Client.hpp"
#include "Server/Server.h"

struct Winsock final{
	Winsock() = default;
	~Winsock();
	_4_DEFAULTED(Winsock)

	int result = 0;

	static const int msgBufferSize = 1024;
	char msgBuffer[msgBufferSize]{};

	WSAData wsaData{};
	fd_set readFDS{};
	fd_set tempFDS{};
	TIMEVAL timeout{};

	std::vector<Client*> clients;
	std::vector<Server*> servers;

	struct InitParams final{
		_6_DEFAULTED(InitParams)

		int portNumber = -999;
		int amtOfServers = 1;
	};

	void Init(const InitParams& params);
};