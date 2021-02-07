#pragma once

#include "Core.hpp"

#include "ProtocolType.hpp"

struct Server final{
	_6_DEFAULTED(Server)

	int portNumber = -999;
	ProtocolType type = ProtocolType::TCP;

	fd_set readFDS{};
	SOCKET mySocket = 0;
	SOCKADDR_IN address{};

	struct InitParams final{
		_6_DEFAULTED(InitParams)

		int portNumber = -999;
		ProtocolType type = ProtocolType::TCP;
	};

	void Init(const InitParams& params);
};