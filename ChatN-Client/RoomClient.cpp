#include "RoomClient.h"
#include "SocketNet.h"
#include "iostream"
using namespace std;
using namespace Protocol;

RoomClient::RoomClient(const std::string& ip, int port, const std::string& username, int roomid)
	:ip(ip),port(port),username(username),roomid(roomid)
{

}

void RoomClient::testConnect()
{
    SocketNet socketNet;
    SOCKET connectSocket = socketNet.getSocket();

    if (!socketNet.tryConnectHost(ip, port)) {
        throw 0;
    }
    socketNet.send(Protocol::Pkg::createGreat());
    unique_ptr<Protocol::Pkg> replyGreet = socketNet.recv();

    if (replyGreet->type != Protocol::PkgType::GREET) {
        throw 0;
    }
}

void RoomClient::login()
{
    if (!sSocket.tryConnectHost(ip, port)) {
        throw 0;
    }
    sSocket.send(Protocol::Pkg::createLogIn(username,roomid));
    unique_ptr<Protocol::Pkg> loginReply=sSocket.recv();

    switch (loginReply->type) {
    default:
        cerr << "unknown server's reply ." << endl;
        throw 0;
    case PkgType::TYPEERR:
        cerr<<"server not support."<< endl;
        throw 0;
    case PkgType::LOGINERR:
        cerr << "server reply error." << endl;
        throw 0;
    case PkgType::LOGINOK:
        string existNames = (char*)loginReply->data;
        cout << "Welcome to the " + to_string(roomid) << " room, all user in the room£º" << endl;
        cout<<existNames<< endl;
        break;
    }


}

std::unique_ptr<Pkg> RoomClient::getPkg()
{
    try {
        return move(sSocket.recv());
    }
    catch(...){
        return std::unique_ptr<Pkg>(nullptr);
    }
}

void RoomClient::sendMsg(std::string msg)
{
    sSocket.send(Protocol::Pkg::createMsg(msg));
}
