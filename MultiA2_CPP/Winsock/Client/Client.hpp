#pragma once

#include "Core.hpp"

struct Client final{ //Passive DS (PDS)/Plain Old DS (PODS)/Plain Old Data (POD)
	_6_DEFAULTED(Client)

	SOCKET mySocket = 0;
	SOCKADDR_IN address{};

	int sizeOfAddress = sizeof(SOCKADDR_IN);

	std::string username = "";
	float colorR = 0.0f;
	float colorG = 0.0f;
	float colorB = 0.0f;
	bool isAfk = false;
};