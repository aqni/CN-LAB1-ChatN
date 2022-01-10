#pragma once
#include<map>
#include<WinSock2.h>
#include<queue>
#include <string>
#include "Protocol.h"
#include "RoomServer.h"

using std::map;
using std::queue;
using std::string;
using std::unique_ptr;

class RoomServer;

class NetEventLoop
{
public:
	NetEventLoop(const string& ip, int port);
	~NetEventLoop();
	void setRoom(RoomServer&);
	void loop();
	void createWriteEvent(SOCKET socket, unique_ptr<Protocol::Pkg>&& pPkg);

private:
	void dealWithRequest(SOCKET socket);
	void closeClient(SOCKET socket);
	void getClient();
	void sendToClient(SOCKET socket, Protocol::Pkg* pPkg);

private:
	bool stop = false;
	RoomServer* pRoomServer;
	map<SOCKET, queue<Protocol::Pkg*>> writeEvents;
	SOCKET listenSocket;
	char buffer[1024];
};

