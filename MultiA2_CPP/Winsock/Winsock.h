#pragma once

#include "Client/Client.hpp"
#include "Server/Server.h"

#include "ObjPool/ObjPool.h"

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

	ObjPool<Client>* clientPool = ObjPool<Client>::GetObjPtr();
	ObjPool<Server>* serverPool = ObjPool<Server>::GetObjPtr();

	std::vector<Client*> clients;
	std::vector<Server*> servers;

	struct InitParams final{
		_6_DEFAULTED(InitParams)

		int portNumber = -999;
		int clientPoolInactiveSize = 999;
		int clientPoolActiveSize = 999;
		int serverPoolInactiveSize = 7;
		int serverPoolActiveSize = 7;
	};

	void Init(const InitParams& params);
	void Run();

	void ActivateClient() const;
	void ActivateServer() const;

	void DeactivateClient(Client* const client) const;
	void DeactivateServer(Server* const server) const;
};