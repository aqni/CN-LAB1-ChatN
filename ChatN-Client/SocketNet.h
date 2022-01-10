#pragma once
#include<string>
#include<chrono>
#include <queue>
#include <WinSock2.h>
#include<memory>
#include "../ChatN-Server/Protocol.h"

class SocketNet
{
public:
	SocketNet();
	~SocketNet();
	static void init();
	bool tryConnectHost(const std::string& ip, int port);
	void send(std::unique_ptr<Protocol::Pkg>&& data);
	std::unique_ptr<Protocol::Pkg> recv();
	SOCKET getSocket() const;
	
private:
	SOCKET socket;
	std::queue<Protocol::Pkg*> packageBuffer;
};

