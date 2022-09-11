#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <array>

#define Default_BUFLEN 512
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

struct ServerInfo
{
	SOCKET socket;
	int id;
	char receivedMsg[Default_BUFLEN];
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
	WSADATA mWsadata;
	SOCKET mServerSocket;
	u_int mDefaultPort = 41902;
	std::array<ClientInfo, 3> mClients;
};

class Client
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static Client& Get();

	bool Startup();

private:
	WSADATA mWsadata;
	u_int mDefaultPort = 41902;
	ServerInfo mServer{ INVALID_SOCKET, -1, "" };
};