#include "RoomServer.h"
#include "Protocol.h"
#include <sstream>
using namespace std;
using namespace Protocol;

RoomServer::RoomServer(NetEventLoop& n)
	:net(n)
{
	n.setRoom(*this);
	n.loop();
}

void RoomServer::addUser(int roomId, SOCKET socket, std::string username)
{
	auto iter = roomMembers.find(roomId);
	if (iter == roomMembers.end()) {
		roomMembers[roomId] = Room{};
		iter= roomMembers.find(roomId);
	}
	Room& room = iter->second;

	if (room.members.find(username) != room.members.end()|| username=="system") { //���������û�
		auto pPkg = Pkg::createLogInErr("Anthor user use the same name.");
		net.createWriteEvent(socket,move(pPkg));
		return;
	}

	room.members.insert(move(make_pair(username, socket)));
	users[socket] = User{roomId ,username };
	ostringstream strBulider; //构建字符串
	for (const auto& pair : room.members) {
		strBulider << pair.first << endl;
	}
	auto pPkg = Pkg::createLogInOK(strBulider.str());
	net.createWriteEvent(socket, move(pPkg));
	//通知所有人
	for (const auto& pair : room.members) {
		auto pPkg = Pkg::createUserUpdate(UserUpdate::UpdateType::ENTER,username);
		net.createWriteEvent(pair.second, move(pPkg));
	}
}

void RoomServer::rmUser(SOCKET socket)
{
	if (users.find(socket) != users.end()) {
		User user = users[socket];
		Room& room = roomMembers[user.roomId];
		room.members.erase(user.name);
		users.erase(socket);
		for (const auto& pair : room.members) {
			auto pPkg = Pkg::createUserUpdate(UserUpdate::UpdateType::LEAVE, user.name);
			net.createWriteEvent(pair.second, move(pPkg));
		}
	}
}

void RoomServer::pushMsg(SOCKET socket, time_t time,string msg)
{
	User& sender = users[socket];
	Room& room = roomMembers[sender.roomId];
	for (const auto& pair : room.members) {
		auto pPkg = Pkg::createPush(time, sender.name,msg);
		net.createWriteEvent(pair.second, move(pPkg));
	}
}
