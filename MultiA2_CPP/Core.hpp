#pragma once

#include <string>
#include <vector>

#ifndef SLN_CONFIG
	#ifdef _DEBUG
		#define SLN_CONFIG (std::string)"Debug"
	#else
		#define SLN_CONFIG (std::string)"Release"
	#endif
#endif

#ifndef SLN_PLAT
	#ifdef _WIN64
		#define SLN_PLAT (std::string)"x64"
	#else
		#define SLN_PLAT (std::string)"x86"
	#endif
#endif

#ifndef _4_DEFAULTED
#define _4_DEFAULTED(...) \
		__VA_ARGS__(const __VA_ARGS__&) = default; \
		__VA_ARGS__(__VA_ARGS__&&) noexcept = default; \
		__VA_ARGS__& operator=(const __VA_ARGS__&) = default; \
		__VA_ARGS__& operator=(__VA_ARGS__&&) noexcept = default;
#endif

#ifndef _6_DEFAULTED
#define _6_DEFAULTED(...) \
		__VA_ARGS__() = default; \
		\
		__VA_ARGS__(const __VA_ARGS__&) = default; \
		__VA_ARGS__(__VA_ARGS__&&) noexcept = default; \
		\
		__VA_ARGS__& operator=(const __VA_ARGS__&) = default; \
		__VA_ARGS__& operator=(__VA_ARGS__&&) noexcept = default; \
		\
		~__VA_ARGS__() = default;
#endif

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")