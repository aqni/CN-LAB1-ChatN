#pragma once
#include<cstdint>
#include<memory>
#include <string>
#include <chrono>


namespace Protocol
{
	using namespace std;
	enum class PkgType :uint16_t
	{
		TYPEERR,    //size=0
		GREET,      //size=0
		LOGIN,      //size=val
		LOGINOK,    //size=val
		LOGINERR,   //size=val
		USER_UPATE, //size=val
		PUSH_MSG,   //size=val
		SEND,       //size=val
		SENDTO,     //size=val
	};
	constexpr uint16_t MAGIC = 0xABCD;
	constexpr uint16_t VERSION = 0;

	struct UserUpdate {
		time_t time;
		enum class UpdateType:uint8_t {
			ENTER,
			LEAVE,
		}type;
		uint8_t strsize;
		char name[];
	};

	struct LogInData {
		int roomId;
		uint8_t strsize;
		char username[];
	};

	struct MsgData {
		time_t time;
		char msg[];
	};

	struct PushData {
		time_t time;
		uint8_t namesize;
		char name_and_msg[];
	};

	struct Pkg
	{
	public:
		static std::unique_ptr<Pkg> createGreat() {
			std::unique_ptr<Pkg> uPtr(new Pkg);
			uPtr->magic = MAGIC;
			uPtr->type = PkgType::GREET;
			uPtr->version = VERSION;
			uPtr->bodySize = 0;
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createTypeErr() {
			std::unique_ptr<Pkg> uPtr(new Pkg);
			uPtr->magic = MAGIC;
			uPtr->type = PkgType::TYPEERR;
			uPtr->version = VERSION;
			uPtr->bodySize = 0;
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createLogIn(const string& username, int roomid) {
			if (username.size() + 1 > UINT8_MAX) throw;
			uint8_t strsize = username.size() + 1;
			int bodysize = sizeof(LogInData) + strsize;
			int memsize = sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::LOGIN;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;
			LogInData* dPtr = (LogInData*)(memPtr->data);
			dPtr->roomId = roomid;
			dPtr->strsize = strsize;
			memcpy_s(dPtr->username, strsize,username.c_str(), strsize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createLogInErr(const string& reason) {
			int bodysize = reason.size()+1;
			int memsize= sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::LOGINERR;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;
			memcpy_s(memPtr->data, bodysize, reason.c_str(), bodysize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createLogInOK(const string& names) {
			int bodysize = names.size() + 1;
			int memsize = sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::LOGINOK;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;
			memcpy_s(memPtr->data, bodysize, names.c_str(), bodysize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createUserUpdate(UserUpdate::UpdateType type,const string& names) {
			if (names.size() + 1 > UINT8_MAX) throw;
			uint8_t strsize = names.size() + 1;
			int bodysize = sizeof(UserUpdate)+ strsize;
			int memsize = sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::USER_UPATE;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;
			UserUpdate* puu = (UserUpdate*)memPtr->data;
			puu->time = chrono::system_clock::to_time_t(chrono::system_clock::now());
			puu->strsize = strsize;
			puu->type = type;
			memcpy_s(puu->name, strsize, names.c_str(), strsize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createMsg(const string& names) {
			int strsize= names.size() + 1;
			int bodysize = strsize +sizeof(MsgData);
			int memsize = sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::SEND;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;

			MsgData* pMsg = (MsgData*)memPtr->data;
			pMsg->time= chrono::system_clock::to_time_t(chrono::system_clock::now());
			memcpy_s(pMsg->msg, strsize, names.c_str(), strsize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}
		static std::unique_ptr<Pkg> createPush(time_t time,const string& name, const string& msg) {
			int namesize = name.size() + 1;
			int msgsize = msg.size() + 1;
			int strsize = msgsize + namesize;
			int bodysize = strsize + sizeof(PushData);
			int memsize = sizeof(Pkg) + bodysize;
			Pkg* memPtr = (Pkg*)new char[memsize];

			memPtr->magic = MAGIC;
			memPtr->type = PkgType::PUSH_MSG;
			memPtr->version = VERSION;
			memPtr->bodySize = bodysize;

			PushData* pMsg = (PushData*)memPtr->data;
			pMsg->time = chrono::system_clock::to_time_t(chrono::system_clock::now());
			pMsg->namesize = namesize;

			memcpy_s(pMsg->name_and_msg, namesize, name.c_str(), namesize);
			memcpy_s(&pMsg->name_and_msg[namesize], msgsize, msg.c_str(), msgsize);

			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}

		static bool comfirmPkg(const Pkg& pkg) {
			return pkg.magic == MAGIC && pkg.version == VERSION;
		}
		static std::unique_ptr<Pkg> copyPkg(const Pkg& pkg,size_t srcSize) {
			int memSize = pkg.size();
			Pkg* memPtr = (Pkg*)new char[memSize];
			memcpy_s(memPtr, memSize,&pkg, srcSize);
			std::unique_ptr<Pkg> uPtr(memPtr);
			return std::move(uPtr);
		}

	public:
		int size() const { return sizeof(Pkg) + bodySize; };
	public:
		uint16_t magic;
		PkgType type;
		uint16_t version;
		uint16_t bodySize;
		uint8_t data[];
	private:
		Pkg()=default;
		Pkg(const Pkg&) = delete;
		Pkg(const Pkg&&) = delete;
	};


};



