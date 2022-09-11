#include "NetworkingComponents.h"

#include <iostream>
#include <string>
#include <XEngine.h>

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

	void processReceives(ServerInfo& server)
	{
		while (true)
		{
			memset(server.receivedMsg, 0, Default_BUFLEN);
			if (server.socket != INVALID_SOCKET)
			{
				int iResult = recv(server.socket, server.receivedMsg, Default_BUFLEN, 0);
				if (iResult != SOCKET_ERROR)
					std::cout << server.receivedMsg << std::endl;
				else
				{
					std::cout << "recv() failed: error = " << WSAGetLastError() << std::endl;
					break;
				}
			}
		} // while 

		if (WSAGetLastError() == WSAECONNRESET)
			std::cout << "The server has disconnected\n";
	}
	
	std::unique_ptr<Server> ServerInstance = nullptr;
	std::unique_ptr<Client> ClientInstance = nullptr;
}

#pragma region Server
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
	sockaddr clientAddr;
	int clientAddLen{ sizeof(clientAddr) };
	std::cout << "Server is listening...\n";
	ZeroMemory(&clientAddr, clientAddLen);
	SOCKET incoming = accept(mServerSocket, (struct sockaddr*)&clientAddr, &clientAddLen);
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
	if (clientAddr.sa_family == AF_INET)
	{
		struct sockaddr_in* s = (struct sockaddr_in*)&clientAddr;
		clientport = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
	}
	else // means ipv6
	{
		struct sockaddr_in6* s = (struct sockaddr_in6*)&clientAddr;
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
		mClients[tmp_id].set(tmp_id, incoming, clientAddr);
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
#pragma endregion

#pragma region Client
void Client::StaticInitialize()
{
	XASSERT(ClientInstance == nullptr, "Client already initialized!");
	ClientInstance = std::make_unique<Client>();
}
void Client::StaticTerminate()
{
	shutdown(ClientInstance.get()->mServer.socket, SD_SEND);

	closesocket(ClientInstance.get()->mServer.socket);
	WSACleanup();

	ClientInstance.reset();
}
Server& Client::Get()
{
	XASSERT(ServerInstance != nullptr, "Client not yet created! Must call StaticInitialize first!");
	return *ServerInstance;
}

bool Client::Startup()
{
	// initializing winsock
	if (WSAStartup(MAKEWORD(2, 2), &mWsadata) != 0)
	{
		errNdie("error starting socket engine\n");
		return false;
	}

	std::cout << "Connecting to server....\n";

	//create socket to connect to server:
	mServer.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mServer.socket == INVALID_SOCKET)
	{
		errNdie("error creating socket \n");
		return false;
	}

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(mDefaultPort);
	if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
	{
		errNdie("inet_pton error\n");
		return false;
	}

	// try to connect to server:
	if (connect(mServer.socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
	{
		closesocket(mServer.socket);
		mServer.socket = INVALID_SOCKET;
		std::cout << "unable to connect to server\n";
		WSACleanup();
		return false;
	}

	//setup interactive mode for the socket:
	const char OPTION_VALUE{ 1 };
	setsockopt(mServer.socket, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int));

	std::cout << "Successfully connected\n";

	// obtain id from server:
	recv(mServer.socket, mServer.receivedMsg, Default_BUFLEN, 0);
	std::string message{ mServer.receivedMsg };
	if (message != "Server is full")
	{
		mServer.id = atoi(mServer.receivedMsg);
		std::thread receiveThread = std::thread(processReceives, std::ref(mServer));
	}
	else
	{
		std::cout << mServer.receivedMsg << std::endl;
		return false;
	}

	return true;
}
#pragma endregion