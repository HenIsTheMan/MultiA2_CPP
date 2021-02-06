#include "Winsock/Winsock.h"

#include <unordered_set>

#include "Pseudorand.h"

int main(const int argc, const char* const* const argv){
	(void)_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	(void)SetWindowPos(
		GetConsoleWindow(),
		0,
		0,
		0,
		GetSystemMetrics(SM_CXFULLSCREEN),
		GetSystemMetrics(SM_CYFULLSCREEN),
		0
	);
	(void)ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	(void)SetLayeredWindowAttributes(GetConsoleWindow(), NULL, 230, LWA_ALPHA);
	(void)SetConsoleTitleA(((std::string)"Console Window (" + SLN_CONFIG + ' ' + SLN_PLAT + ')').c_str());

	PHANDLER_ROUTINE ConsoleEventHandler = [](const DWORD consoleEvent)->BOOL{
		LPCWSTR msg;
		switch(consoleEvent){
			case CTRL_C_EVENT:
				msg = L"Ctrl + C";
				break;
			case CTRL_BREAK_EVENT:
				msg = L"Ctrl + BREAK";
				break;
			case CTRL_CLOSE_EVENT:
				msg = L"Closing prog...";
				break;
			case CTRL_LOGOFF_EVENT:
			case CTRL_SHUTDOWN_EVENT:
				msg = L"User is logging off...";
				break;
			default:
				msg = L"???";
		}

		MessageBox(NULL, msg, L"Msg", MB_OK);
		return TRUE;
	};
	(void)SetConsoleCtrlHandler(ConsoleEventHandler, TRUE);

    Winsock* winsock = new Winsock();
    Winsock::InitParams params;

    params.clientPoolInactiveSize = 999;
    params.clientPoolActiveSize = 999;
    params.serverPoolInactiveSize = 7;
    params.serverPoolActiveSize = 7;

    winsock->Init(params);

    Server* const server = winsock->ActivateServer();
	static std::unordered_set<int> portNumbers = {};

    if(argc == 2){
        server->Init({
            atoi(argv[1])
        });
    } else{
		int val =  0;
		do{
			val = PseudorandMinMax(1000, 9999);
		} while(portNumbers.find(val) != portNumbers.end());

        server->Init({
            val
        });
		portNumbers.insert(val); 
    }

    winsock->Run();

    if(winsock != nullptr){
        delete winsock;
        winsock = nullptr;
    }
}