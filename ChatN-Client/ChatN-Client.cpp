// ChatN-Client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <exception>
#include <string>
#include "CLIUI.h"
#include "RoomClient.h"
#include "ChatN-Client.h"
using namespace std;

int main(int argc,char* argv[])
{
	//获取参数
	if (argc == 1) {
		cerr<<"Please input arguments :<ip> <port> <yourname> <roomid>" << endl;
		return 1;
	}
	string ip, username;
	int port, roomid;
	try {
		ip = argv[1];
		port = stoi(argv[2]);
		username = argv[3];
		roomid = stoi(argv[4]);
	}
	catch (...) {
		cerr<<"arguments error!"<< endl;
		return 1;
	}

	//测试连通性
	RoomClient rc(ip, port, username, roomid);
	try {
		rc.testConnect();
	}
	catch (...) {
		cerr << "failed to connect!" << endl;
		return 1;
	}

	//尝试连接
	try {
		rc.login();
	}
	catch (...) {
		cerr << "login err!" << endl;
		return 1;
	}

	auto ui=CLIUI(rc);
	ui.exec();

}

