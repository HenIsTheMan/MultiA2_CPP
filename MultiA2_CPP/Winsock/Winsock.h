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
	fd_set tempFDS{};
	TIMEVAL timeout{};

	ObjPool<Client>* clientPool = ObjPool<Client>::GetObjPtr();
	ObjPool<Server>* serverPool = ObjPool<Server>::GetObjPtr();

	const std::vector<Server*>& activeServers = serverPool->GetActiveObjs();
	const std::vector<Client*>& activeClients = clientPool->GetActiveObjs();

	struct InitParams final{
		_6_DEFAULTED(InitParams)

		int clientPoolInactiveSize = 999;
		int clientPoolActiveSize = 999;
		int serverPoolInactiveSize = 4;
		int serverPoolActiveSize = 4;
		long timeoutSec = 4;
		long timeoutMicrosec = 0;
	};

	void Init(const InitParams& params);
	void Run();
	void RunChiefProcess(Server* const server);

	void OnClientConnected(Server* const server);
	void OnClientDisconnected(Server* const server, SOCKET& currSocket);
	void ProcessRS(Server* const server, SOCKET& currSocket);

	Client* ActivateClient() const;
	Server* ActivateServer() const;

	void DeactivateClient(Client* const client) const;
	void DeactivateServer(Server* const server) const;
};