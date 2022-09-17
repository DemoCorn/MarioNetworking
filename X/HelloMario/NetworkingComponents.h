#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <array>
#include <string>
#include <mutex>
#include <vector>
#include "GameState.h"

#define Default_BUFLEN 512
const int MAX_CLIENTS{ 3 };

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

class NetworkingComponent
{
public:
	void Set(GameState* state);
	bool virtual Startup() = 0;
	bool virtual Send(std::string message) = 0;

	static const void Send(std::string message, bool isServer);

	std::vector<std::string> messages;

	std::mutex m;

protected:
	std::vector<std::string> backlog;

	GameState* mGameState;
	WSADATA mWsadata;
	u_int mDefaultPort = 41902;
};

class Server : public NetworkingComponent
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static Server& Get();

	bool virtual Send(std::string message);

	void process_client(int id);

	void Connect();
	bool virtual Startup();

	int GetClientCount() { for (int i = 0; i < mClients.size(); i++) { if (mClients[i].id == -1) return i;} return mClients.size(); }

	bool GetNewPlayer();
	int GetCurrentPlayer() { return currentPlayer; }

private:
	int currentPlayer = 0;
	SOCKET mServerSocket;
	std::array<ClientInfo, 3> mClients;
	bool mConnecting;
};

class Client : public NetworkingComponent
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static Client& Get();

	bool virtual Send(std::string message);
	void processReceives();
	bool virtual Startup();

	int GetClientID() { return mServer.id; }

	void SetIP(std::string ip) { mIp = ip; }

private:
	ServerInfo mServer{ INVALID_SOCKET, -1, "" };
	std::thread mReceiveThread;
	std::string mIp;
};