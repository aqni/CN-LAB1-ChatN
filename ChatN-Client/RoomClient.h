#pragma once


#include<string>
#include<winsock2.h>
#include"SocketNet.h"
#include "../ChatN-Server/Protocol.h"
#include <sstream>

class RoomClient
{
public:
	RoomClient(const std::string& ip, int port, const std::string& username, int roomid);

	void testConnect();
	void login();
	std::unique_ptr<Protocol::Pkg> getPkg();
	int getRoomId() const {return roomid;};
	const std::string& getUserName() const { return username; };
	void sendMsg(std::string msg);
private:
	const std::string ip;
	const int port;
	const std::string username;
	const int roomid;
	SocketNet sSocket;
};

