#pragma once

#include "Core.hpp"

struct Server final{
	_6_DEFAULTED(Server)

	int portNumber = 2169;

	SOCKET socket = 0;
	SOCKADDR_IN address{};

	struct InitParams final{
		_6_DEFAULTED(InitParams)

			int portNumber = -999;
	};

	void Init(const InitParams& params);
};