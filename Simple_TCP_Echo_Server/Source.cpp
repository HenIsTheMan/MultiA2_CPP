#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdio.h>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main(const int, const char* const* const){
    const int bufferSize = 1024;
    const int portNumber = 9876;

    WSADATA wsaData{};
    SOCKET serverSocket = 0;
    SOCKET clientSocket = 0;
    sockaddr_in serverAddress{};
    sockaddr_in clientAddress{};
    char msgBuffer[bufferSize]{};
    int sizeOfClientAddress = sizeof(clientAddress);
    int result = 0;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != NO_ERROR){
        (void)printf("WSAStartup failed with error %d\n", result);
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET){
        (void)printf("socket failed with error %d\n", WSAGetLastError());
        (void)WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR){
        (void)printf("bind failed with error %d\n", WSAGetLastError());
        (void)closesocket(serverSocket);
        (void)WSACleanup();
        return 1;
    }

    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR){
        (void)printf("listen function failed with error %d\n", WSAGetLastError());
        (void)closesocket(serverSocket);
        (void)WSACleanup();
        return 1;
    }

    (void)printf("[TCP Echo Server] Waiting for client to connect...\n");
    result = SOCKET_ERROR;

    for(;;){
        if(result == SOCKET_ERROR){
            clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &sizeOfClientAddress);
            if(clientSocket == INVALID_SOCKET){
                (void)printf("accept failed with error %ld\n", WSAGetLastError());
                (void)closesocket(serverSocket);
                (void)WSACleanup();
                return 1;
            } else{
                (void)system("cls");
                (void)printf("[TCP Echo Server] Client (%d.%d.%d.%d: %d) connected...\n\n",
                    clientAddress.sin_addr.S_un.S_un_b.s_b1,
                    clientAddress.sin_addr.S_un.S_un_b.s_b2,
                    clientAddress.sin_addr.S_un.S_un_b.s_b3,
                    clientAddress.sin_addr.S_un.S_un_b.s_b4,
                    ntohs(clientAddress.sin_port)
                );

                //* Send welcome msg
                char welcomeMsgBuffer[] = "Welcome to Simple TCP/IP Echo Server";
                const size_t welcomeMsgBufferSize = sizeof(welcomeMsgBuffer) / sizeof(welcomeMsgBuffer[0]);
                result = send(clientSocket, welcomeMsgBuffer, welcomeMsgBufferSize, 0);
                //*/
            }
        }

        memset(msgBuffer, '\0', bufferSize);
        result = recv(clientSocket, msgBuffer, bufferSize, 0);
        //result = recv(clientSocket, msgBuffer, 2, 0);

        if(result > 0){
            const std::string msg(msgBuffer);
            const size_t msgLen = msg.length();
            const size_t pos = msg.find(' ');

            if(msgBuffer[0] == '/' && pos != std::string::npos && pos + 1 < msgLen && msg.substr(1, 3) == "msg"){
                memset(msgBuffer, '\0', bufferSize);
                const std::string substr = msg.substr(pos + 1, msgLen - (pos + 1));
                const int substrLen = (int)substr.length();

                for(int i = 0; i < substrLen; ++i){
                    msgBuffer[i] = substr[i];
                }

                (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n",
                    msgBuffer,
                    clientAddress.sin_addr.S_un.S_un_b.s_b1,
                    clientAddress.sin_addr.S_un.S_un_b.s_b2,
                    clientAddress.sin_addr.S_un.S_un_b.s_b3,
                    clientAddress.sin_addr.S_un.S_un_b.s_b4,
                    ntohs(clientAddress.sin_port),
                    result
                );

                char customMsgBuffer[] = "Command received :)";
                const size_t customMsgBufferSize = sizeof(customMsgBuffer) / sizeof(customMsgBuffer[0]);
                result = send(clientSocket, customMsgBuffer, customMsgBufferSize, 0);
            } else{
                (void)printf("\"%s\" (from %d.%d.%d.%d: %d, bytes read: %d)\n",
                    msgBuffer,
                    clientAddress.sin_addr.S_un.S_un_b.s_b1,
                    clientAddress.sin_addr.S_un.S_un_b.s_b2,
                    clientAddress.sin_addr.S_un.S_un_b.s_b3,
                    clientAddress.sin_addr.S_un.S_un_b.s_b4,
                    ntohs(clientAddress.sin_port),
                    result
                );

                char customMsgBuffer[] = "Msg received :)";
                const size_t customMsgBufferSize = sizeof(customMsgBuffer) / sizeof(customMsgBuffer[0]);
                result = send(clientSocket, customMsgBuffer, customMsgBufferSize, 0);
            }
        } else if(result == SOCKET_ERROR){
            (void)system("cls");
            (void)printf("[TCP Echo Server] Client (%d.%d.%d.%d: %d) disconnected...\n\n",
                clientAddress.sin_addr.S_un.S_un_b.s_b1,
                clientAddress.sin_addr.S_un.S_un_b.s_b2,
                clientAddress.sin_addr.S_un.S_un_b.s_b3,
                clientAddress.sin_addr.S_un.S_un_b.s_b4,
                ntohs(clientAddress.sin_port)
            );
            continue;
        } else{
            result == 0 ? (void)printf("Connection closed\n") : (void)printf("recv failed with error %d\n", WSAGetLastError());
        }

        result = send(clientSocket, msgBuffer, result, 0);
        if(result == SOCKET_ERROR){
            //(void)printf("send failed with error %d\n", WSAGetLastError());
            //break;
            (void)system("cls");
            (void)printf("[TCP Echo Server] Client (%d.%d.%d.%d: %d) disconnected...\n\n",
                clientAddress.sin_addr.S_un.S_un_b.s_b1,
                clientAddress.sin_addr.S_un.S_un_b.s_b2,
                clientAddress.sin_addr.S_un.S_un_b.s_b3,
                clientAddress.sin_addr.S_un.S_un_b.s_b4,
                ntohs(clientAddress.sin_port)
            );
            continue;
        } else{
            (void)printf("Sent \"%s\" back (to %d.%d.%d.%d: %d, bytes sent: %d)\n\n",
                msgBuffer,
                clientAddress.sin_addr.S_un.S_un_b.s_b1,
                clientAddress.sin_addr.S_un.S_un_b.s_b2,
                clientAddress.sin_addr.S_un.S_un_b.s_b3,
                clientAddress.sin_addr.S_un.S_un_b.s_b4,
                ntohs(clientAddress.sin_port),
                result
            );
        }
    }

    (void)closesocket(clientSocket);
    (void)closesocket(serverSocket);
    (void)WSACleanup();
}