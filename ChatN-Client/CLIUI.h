#pragma once
#include<string>
#include "RoomClient.h"
#include "../ChatN-Server/Protocol.h"


using std::string;
class RoomClient;

class CLIUI
{
public:
	CLIUI(RoomClient& rc);
	void exec() noexcept(false);
private:
	string getInput();
	void putInfo(const string& info);
	void processPkg(std::unique_ptr<Protocol::Pkg> pPkg);
	void processInput(std::string input);
private:
	RoomClient& roomClient;
	string roomName;
	std::ostringstream chatRecord; //»º´æÁÄÌì¼ÇÂ¼
	const string inputPrompt = ">>";
};

