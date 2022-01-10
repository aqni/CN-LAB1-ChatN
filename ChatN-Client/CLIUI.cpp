#include "CLIUI.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <map>
#include <thread>
#include <sstream>
#include <ctime>

#include "SocketNet.h"

using namespace std;
using namespace  Protocol;

const string helpInfo = R"(this is some help info
	.HELP
	.QUIT
	.SEND <message>
	.SENDTO <name> <message>
)";

CLIUI::CLIUI(RoomClient& rc)
	:roomClient(rc)
{
	putInfo("Input \".help\" for help.");
}

string CLIUI::getInput()
{
	cout << this->inputPrompt;
	string s;
	getline(cin, s);
	return s;
}

void CLIUI::putInfo(const string& info)
{
	cout << roomName << info << endl;
}


void CLIUI::exec()
{
	thread recvThread([this]() {
		while (true) {
			auto pPkg = roomClient.getPkg();
			if (pPkg.get() == nullptr) break;
			processPkg(move(pPkg));
		}
		cerr << "recv exit!" << endl;
	});

	while (true) {
		string input = getInput();
		processInput(move(input));
	}
}


void CLIUI::processPkg(std::unique_ptr<Protocol::Pkg> pPkg)
{
	ostringstream &oss=chatRecord;
	struct tm stime;
	const UserUpdate* pUserUpdate = (const UserUpdate*)pPkg->data;
	const PushData* pPush=(const PushData*)pPkg->data;
	switch (pPkg->type) {
	default:
		cerr<<"Recv a unknow package!" << endl;
		break;
	case PkgType::USER_UPATE:
		localtime_s(&stime, &(pUserUpdate->time));
		oss << "|" << std::put_time(&stime, "%Y-%m-%d %H:%M:%S") << "|";
		oss << "[" << pUserUpdate->name << "]: ";
		switch (pUserUpdate->type) {
		default:throw;
		case UserUpdate::UpdateType::ENTER:
			oss << "enter this room.";
			break;
		case UserUpdate::UpdateType::LEAVE:
			oss << "leave this room.";
			break;
		}
		break;
	case PkgType::PUSH_MSG:
		localtime_s(&stime, &(pPush->time));
		oss <<"|" << std::put_time(&stime, "%Y-%m-%d %H:%M:%S") << "|";
		oss <<"[" << pPush->name_and_msg << "]: ";
		oss << &pPush->name_and_msg[pPush->namesize];
		break;
	}
	oss << endl;
}

enum class opEnum {
	QUIT,
	HELP,
	SEND,
	SENDTO,
	SHOW,
};

static map<string, opEnum> str2Enum = {
	{".HELP",opEnum::HELP},
	{".QUIT",opEnum::QUIT},
	{".SEND",opEnum::SEND},
	{".SENDTO",opEnum::SENDTO},
	{".",opEnum::SHOW},
};

void CLIUI::processInput(std::string input)
{
	if (!input.empty() && input[0] == '.') {
		int idx = input.find(' ');
		string opStr = input.substr(0, idx);
		string body = input.substr(idx+1);
		if (str2Enum.count(opStr) > 0) {
			switch (str2Enum[opStr]) {
			default:break;
			case opEnum::QUIT:
				exit(0);
				break;
			case opEnum::HELP:
				putInfo(helpInfo);
				break;
			case opEnum::SEND:
				roomClient.sendMsg(body);
				break;
			case opEnum::SENDTO:
				throw 1;
			case opEnum::SHOW:
				cout<<chatRecord.str()<<endl;
				break;
			}
		}
		else {
			putInfo("unknow op: " + opStr);
		}
	}
	else {
		putInfo("ECHO: " + input);
	}
}