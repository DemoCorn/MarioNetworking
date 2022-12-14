#include "NetworkingComponents.h"

#include <iostream>
#include <string>
#include <XEngine.h>

#pragma comment(lib, "Ws2_32.lib")

void errNdie(const char* msg)
{
	std::cout << msg << std::endl;
}

void NetworkingComponent::Set(GameState* state)
{
	mGameState = state;
}

const void NetworkingComponent::Send(std::string message, bool isServer)
{
	if (isServer)
	{
		Server::Get().Send(message);
	}
	else
	{
		Client::Get().Send(message);
	}
}

std::unique_ptr<Server> ServerInstance = nullptr;
std::unique_ptr<Client> ClientInstance = nullptr;

#pragma region Server
bool Server::Send(std::string message)
{
	mConnecting = false;
	for (int i = 0; i < mClients.size(); ++i)
	{
		if (mClients[i].socket != INVALID_SOCKET)
		{
			send(mClients[i].socket, message.c_str(), strlen(message.c_str()), 0);
		}
	}
	return true;
}

void Server::process_client(int id)
{
	std::unique_lock<std::mutex> lock{ m, std::defer_lock };
	std::string msg{};
	char tmpmsg[Default_BUFLEN]{ "" };

	ClientInfo& client = mClients[id];
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
				{
					if (lock.try_lock())
					{
						for (std::string message : backlog)
						{
							messages.push_back(message);
						}
						backlog.clear();
						messages.push_back(tmpmsg);
						lock.unlock();
					}
					else
					{
						backlog.push_back(tmpmsg);
					}

					//broadcast the msg to the other clients:
					for (int i = 0; i < mClients.size(); ++i)
					{
						if (mClients[i].socket != INVALID_SOCKET && client.id != i)
							send(mClients[i].socket, tmpmsg, strlen(tmpmsg), 0);
					}
				}
			}
			else // means this client socket is not alive anymore
			{
				msg = "Client #" + std::to_string(client.id) + " Disconnected";
				std::cout << msg << std::endl;
				closesocket(client.socket);
				mClients[client.id].socket = INVALID_SOCKET;

				// broadcast the disconnection messsage to the other clients
				for (int i = 0; i < mClients.size(); ++i)
				{
					if ((mClients[i].socket != INVALID_SOCKET))
						send(mClients[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}

				break; // stop chat session for this client
			}
		}
	}
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

	mConnecting = true;
	std::thread connect = std::thread(&Server::Connect, this);
	connect.detach();

	return true;
}

void Server::Connect()
{
	while (mConnecting)
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
			return;
		}
		std::cout << "Accepted a  client\n";

		// find the first clientinfo object with id == -1, to use for this new client.
		int tmp_id{ -1 };
		for (int i = 0; i < mClients.size(); ++i)
		{
			if (mClients[i].socket == INVALID_SOCKET)
			{
				tmp_id = i;
				break;
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
			mClients[tmp_id].th = std::thread(&Server::process_client, this, tmp_id);
		}
		else
		{
			std::string msg("Server is full. Try connecting later");
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			closesocket(incoming);
		}
	}
}

bool Server::GetNewPlayer()
{
	currentPlayer = (currentPlayer + 1) % (GetClientCount() + 1);
	std::string message{ "P " + std::to_string(currentPlayer - 1) };
	Send(message);

	if (currentPlayer == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
#pragma endregion

#pragma region Client

bool Client::Send(std::string message)
{
	if (send(mServer.socket, message.c_str(), strlen(message.c_str()), 0) < 0)
	{
		std::cout << "send() failed " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

void Client::processReceives()
{
	std::unique_lock<std::mutex> lock{ m, std::defer_lock };
	while (true)
	{
		memset(mServer.receivedMsg, 0, Default_BUFLEN);
		if (mServer.socket != INVALID_SOCKET)
		{
			int iResult = recv(mServer.socket, mServer.receivedMsg, Default_BUFLEN, 0);
			if (iResult != SOCKET_ERROR)
			{
				std::string recievedMessage = std::string(mServer.receivedMsg);
				if (recievedMessage == "Play")
				{
					*mGameState = GameState::Play;
				}
				else
				{
					if (lock.try_lock())
					{
						for (std::string message : backlog)
						{
							messages.push_back(message);
						}
						backlog.clear();
						messages.push_back(recievedMessage);
						lock.unlock();
					}
					else
					{
						backlog.push_back(recievedMessage);
					}
				}
			}
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
Client& Client::Get()
{
	XASSERT(ClientInstance != nullptr, "Client not yet created! Must call StaticInitialize first!");
	return *ClientInstance;
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
	if (inet_pton(AF_INET, mIp.c_str(), &server_address.sin_addr) <= 0)
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
		mReceiveThread = std::thread(&Client::processReceives, this);
	}
	else
	{
		std::cout << mServer.receivedMsg << std::endl;
		return false;
	}

	return true;
}
#pragma endregion