#include "NetEvent.h"
#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "Protocol.h"

using namespace std;
using namespace Protocol;
#pragma comment(lib, "Ws2_32.lib")

NetEventLoop::NetEventLoop(const string& ip, int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		std::cerr << "WSAStartup err!" << std::endl;
		throw;
	};

	this->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		std::cerr << "socket() err:" << WSAGetLastError() << std::endl;
		throw;
	}

	//bind
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);  //inet_aton vs���� ���õĺ���
	addr.sin_port = htons(port);
	if (bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind err:" << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		throw;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen err:" << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		throw;
	}

	cout << "Listened in" + ip + ":" + to_string(port) << endl;
	writeEvents[listenSocket] = queue<Pkg*>();
	//unsigned long on = 1;
	//ioctlsocket(listenSocket, FIONBIO, &on);
}

NetEventLoop::~NetEventLoop()
{
	closesocket(listenSocket);

	for (auto& pair : writeEvents) {
		closesocket(pair.first);
		while (!pair.second.empty()) {
			delete pair.second.front();
			pair.second.pop();
		}
	}

	WSACleanup();
}

void NetEventLoop::setRoom(RoomServer& p)
{
	if (pRoomServer != nullptr) {
		delete pRoomServer;
	}
	pRoomServer = &p;
}


void NetEventLoop::loop()
{
	fd_set readFds, writeFds, expFds;
	timeval tv = { 0,10000 };

	while (!stop) {
		FD_ZERO(&readFds);
		FD_ZERO(&writeFds);

		for (const auto& pair : writeEvents) {
			FD_SET(pair.first, &readFds);
			if (pair.first != listenSocket)
				FD_SET(pair.first, &writeFds);
		}

		select(0, &readFds, &writeFds, NULL, NULL);
		for (int i = 0; i < readFds.fd_count; i++) {
			if (readFds.fd_array[i] == listenSocket) {
				getClient();
			}
			else {
				dealWithRequest(readFds.fd_array[i]);
			}
		}

		for (int i = 0; i < writeFds.fd_count; i++) {
			SOCKET clientSocket = readFds.fd_array[i];
			if (writeEvents.find(clientSocket) != writeEvents.end()) {
				queue<Pkg*>& writeQueue = writeEvents[clientSocket];
				while (!writeQueue.empty()) {
					Pkg* pPkg = writeQueue.front();
					sendToClient(clientSocket, pPkg);
					delete pPkg;
					writeQueue.pop();
				}
			}
		}
	}

}

void NetEventLoop::createWriteEvent(SOCKET socket, unique_ptr<Pkg>&& pPkg)
{
	auto writeQueueIter = writeEvents.find(socket);
	if (writeQueueIter == writeEvents.end()) throw;

	writeQueueIter->second.push(pPkg.release());
}

void NetEventLoop::sendToClient(SOCKET socket, Pkg* pPkg)
{
	cout << "send"<<to_string((int)socket)<<endl;
	int n = ::send(socket, (const char*)pPkg, pPkg->size(),0);
	if (n != pPkg->size()) {
		cerr<< WSAGetLastError ()<<endl;
		throw;
	};
}

void NetEventLoop::closeClient(SOCKET socket)
{
	writeEvents.erase(socket);
	closesocket(socket);
	pRoomServer->rmUser(socket);
	cout << "connect close" << endl;
}

void NetEventLoop::getClient()
{
	sockaddr_in clientAddr;
	int len = sizeof(clientAddr);
	SOCKET clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		throw;
	}

	writeEvents[clientSocket] = queue<Pkg*>();
	cout << "Accept a connect:"+to_string((int)clientSocket) << endl;
}

void NetEventLoop::dealWithRequest(SOCKET socket)
{
	cout << "deal "+ to_string(socket) << endl;
	int n = ::recv(socket, buffer, sizeof(buffer), 0);
	if (n == 0) {
		cout<<"gracefully closed" << endl;
		return closeClient(socket);
	}
	if (n == SOCKET_ERROR) {
		cout << "err closed" << endl;
		return closeClient(socket);
	}
	Pkg* pPkg = (Pkg*)buffer;
	if (!Pkg::comfirmPkg(*pPkg)) throw;
	auto pNewPkg = Pkg::copyPkg(*pPkg, n);
	if (n != pPkg->size()) {
		if (n < pPkg->size()) {
			throw;
		}
		else throw;
	}

	switch (pPkg->type) {
	default:
		cout << "type err" << endl;
		createWriteEvent(socket, Pkg::createTypeErr());
		break;
	case PkgType::GREET:
		cout << "recv greet" << endl;
		createWriteEvent(socket, Pkg::createGreat());
		break;
	case PkgType::LOGIN:
		pRoomServer->addUser(((LogInData*)(pNewPkg->data))->roomId,socket,string(((LogInData*)(pNewPkg->data))->username));
		cout << string(((LogInData*)(pNewPkg->data))->username) <<" login. " <<"Enter room:"<< ((LogInData*)(pNewPkg->data))->roomId <<endl;
		break;
	case PkgType::SEND:
		pRoomServer->pushMsg(socket, ((MsgData*)pPkg->data)->time,string((char*)((MsgData*)pPkg->data)->msg));
		break;
	}

}