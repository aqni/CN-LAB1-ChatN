#include "SocketNet.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <memory>
#include <iostream>
#include <queue>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

void SocketNet::init()
{
    static bool inited = false;
    if (!inited) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
            throw;
        }
        inited = true;
    }
}

SOCKET SocketNet::getSocket() const
{
    return socket;
}

SocketNet::~SocketNet()
{
    while (!packageBuffer.empty()) {
        delete packageBuffer.front();
        packageBuffer.pop();
    }
    closesocket(socket);
}

SocketNet::SocketNet()
{
    init();
    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) throw;
}

bool SocketNet::tryConnectHost(const std::string& ip, int port)
{
    sockaddr_in hostaddr;
    hostaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &hostaddr.sin_addr.s_addr);  //inet_aton vs报错 弃用的函数
    hostaddr.sin_port = htons(port);
    int ret = connect(socket, (struct sockaddr*)&hostaddr, sizeof(hostaddr));

    return ret == 0;
}

void SocketNet::send(unique_ptr<Protocol::Pkg>&& pkg)
{
    int n=::send(socket, (const char*)pkg.get(), pkg->size(), 0);
    if (n != pkg->size()) throw;
}


constexpr int BUF_SIZE = 1024;
char buffer[BUF_SIZE];
using namespace Protocol;

std::unique_ptr<Pkg> SocketNet::recv()
{
    if (packageBuffer.empty()) {
        int n = ::recv(socket, buffer, BUF_SIZE, 0);
        if (n <= 0) throw 0;
        int idx = 0;
        while (idx < n) { //分包
            Protocol::Pkg* ptr = (Protocol::Pkg*)&buffer[idx];
            idx += ptr->size();
            if (idx > n) throw;//包收一半
            if (!Pkg::comfirmPkg(*ptr)) throw;
            char* mem = new char[ptr->size()];
            memcpy(mem, buffer, ptr->size());
            packageBuffer.push((Pkg*)mem);
        }
    }
    Pkg* mem = packageBuffer.front();
    packageBuffer.pop();
    unique_ptr<Pkg> uPtr(mem);
    return move(uPtr);
    
}
