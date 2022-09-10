#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <array>

const int MAX_CLIENTS{ 10 };

struct ClientInfo
{
	int id = -1;
	SOCKET socket;
	sockaddr address;
	std::thread th;
	ClientInfo() : id(-1), socket(INVALID_SOCKET) {}
	void set(int i, SOCKET s, sockaddr a)
	{
		id = i;
		socket = s;
		address = a;
	}

};

class Server
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static Server& Get();

	bool Startup();
	bool Connect();

	int GetClientCount() { for (int i = 0; i < mClients.size(); i++) { if (mClients[i].id == -1) return i;} return mClients.size(); }

private:
	u_int mDefaultPort = 41902;
	std::array<ClientInfo, 3> mClients;
	SOCKET mServerSocket;
	sockaddr mClientAddr;
	int mClientAddLen{ sizeof(mClientAddr) };
	WSADATA mWsadata;
};