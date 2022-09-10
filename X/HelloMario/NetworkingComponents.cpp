#include "NetworkingComponents.h"

#include <iostream>
#include <string>
#include <XEngine.h>

#define Default_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")

namespace
{
	void errNdie(const char* msg)
	{
		std::cout << msg << std::endl;
	}

	void process_client(int id, std::array<ClientInfo, MAX_CLIENTS>& clients)
	{
		std::string msg{};
		char tmpmsg[Default_BUFLEN]{ "" };

		ClientInfo& client = clients[id];
		// client chat session
		while (true)
		{
			memset(tmpmsg, 0, Default_BUFLEN);
			if (client.socket != INVALID_SOCKET)
			{
				int iRes = recv(client.socket, tmpmsg, Default_BUFLEN, 0);
				if (iRes != SOCKET_ERROR)
				{
					if (strcmp("", tmpmsg) != 0)
						msg = "Client #" + std::to_string(client.id) + ": " + tmpmsg;
					std::cout << msg << std::endl;

					//broadcast the msg to the other clients:
					for (int i = 0; i < clients.size(); ++i)
					{
						if (clients[i].socket != INVALID_SOCKET && client.id != i)
							send(clients[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
					}
				}
				else // means this client socket is not alive anymore
				{
					msg = "Client #" + std::to_string(client.id) + " Disconnected";
					std::cout << msg << std::endl;
					closesocket(client.socket);
					clients[client.id].socket = INVALID_SOCKET;

					// broadcast the disconnection messsage to the other clients
					for (int i = 0; i < MAX_CLIENTS; ++i)
					{
						if ((clients[i].socket != INVALID_SOCKET))
							send(clients[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
					}

					break; // stop chat session for this client
				}
			}
		}
	}
	
	std::unique_ptr<Server> ServerInstance = nullptr;
}

void Server::StaticInitialize()
{
	XASSERT(ServerInstance == nullptr, "Sever already initialized!");
	ServerInstance = std::make_unique<Server>();
}
void Server::StaticTerminate()
{
	for (int i = 0; i < ServerInstance.get()->mClients.size(); ++i)
	{
		if (ServerInstance.get()->mClients[i].id != -1)
		{
			ServerInstance.get()->mClients[i].th.join();
			closesocket(ServerInstance.get()->mClients[i].socket);
		}
	}

	closesocket(ServerInstance.get()->mServerSocket);

	WSACleanup();

	ServerInstance.reset();
}
Server& Server::Get()
{
	XASSERT(ServerInstance != nullptr, "Server not yet created! Must call StaticInitialize first!");
	return *ServerInstance;
}


bool Server::Startup()
{
	if (WSAStartup(MAKEWORD(2, 2), &mWsadata) != 0)
	{
		errNdie("WSAStartup failed!");
		return false;
	}


	// let's create our listening server
	mServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mServerSocket == INVALID_SOCKET)
	{
		errNdie("socket creation failed!!");
		return false;
	}

	// set socket options
	const char VALUE{ 1 };
	setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &VALUE, sizeof(int));  // make it possible to reuse the addresses that have not been used for last 2 minutes
	setsockopt(mServerSocket, IPPROTO_TCP, TCP_NODELAY, &VALUE, sizeof(int));  // instruct the transport layer to send every message immediately

	DWORD nonBlocking{ 1 };
	if (ioctlsocket(mServerSocket, FIONBIO, &nonBlocking) != 0)
	{
		std::cout << "ioctlsocket failed with error " << WSAGetLastError() << std::endl;
		return false;
	}

	// assign address to our listening socket
	struct sockaddr_in servaddr;
	ZeroMemory(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(mDefaultPort);

	std::cout << "Now binding server socket ...\n";
	if (bind(mServerSocket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		errNdie("cannot connect");
		return false;
	}

	if (listen(mServerSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		errNdie("failed to listen on server socket");
		return false;
	}

	return true;
}

bool Server::Connect()
{
	std::cout << "Server is listening...\n";
	ZeroMemory(&mClientAddr, mClientAddLen);
	SOCKET incoming = accept(mServerSocket, (struct sockaddr*)&mClientAddr, &mClientAddLen);
	if (incoming == INVALID_SOCKET)
	{
		std::cout << "accept reported invalid socket: error code is " << WSAGetLastError() <<
			"We ignore it for now.\n";
		return false;
	}
	std::cout << "Accepted a  client\n";

	// find the first clientinfo object with id == -1, to use for this new client.
	int tmp_id{ -1 };
	for (int i = 0; i < mClients.size(); ++i)
	{
		if (mClients[i].socket == INVALID_SOCKET)
		{
			tmp_id = i;
			return false;
		}
	}

	// extract ip address and port of the new client: deal with both IPV4 and IPV6
	char ipstr[INET6_ADDRSTRLEN];
	int clientport{};
	if (mClientAddr.sa_family == AF_INET)
	{
		struct sockaddr_in* s = (struct sockaddr_in*)&mClientAddr;
		clientport = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
	}
	else // means ipv6
	{
		struct sockaddr_in6* s = (struct sockaddr_in6*)&mClientAddr;
		clientport = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
	}
	std::cout << "Client ip address: " << ipstr << ", client port: " << clientport << std::endl;
	if (tmp_id != -1)
	{
		// send the id to the client:
		std::string msg{ std::to_string(tmp_id) };
		send(incoming, msg.c_str(), strlen(msg.c_str()), 0);

		// add the client to the clientinfo list:
		mClients[tmp_id].set(tmp_id, incoming, mClientAddr);
		//mClients[tmp_id].th = std::thread(process_client, tmp_id, std::ref(mClients));
	}
	else
	{
		std::string msg("Server is full. Try connecting later");
		send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
		closesocket(incoming);
	}
	return true;
}
