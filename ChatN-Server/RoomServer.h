#pragma once
#include<map>
#include<vector>
#include<list>
#include<string>
#include<WinSock2.h>
#include"NetEvent.h"

class NetEventLoop;

struct Room {
	std::map<std::string, SOCKET> members;
};

struct User {
	int roomId;
	std::string name;
};

class RoomServer
{
public:
	RoomServer(NetEventLoop& n);
	void addUser(int roomId,SOCKET socket,std::string username);
	void rmUser(SOCKET socket);
	void pushMsg(SOCKET socket, time_t time, std::string msg);
private:
	NetEventLoop& net;
	std::map<int, Room> roomMembers;
	std::map<SOCKET, User> users;
};

