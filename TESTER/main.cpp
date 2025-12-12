#include <iostream>
#include "Neses/NesesString.hpp"
#include "Neses/Exporter.h"
#include "Neses/App.hpp"
#include "Neses/Logger.hpp"
#include <windows.h>




class MyApp : public NESES::AppImplBase
{
public:
	virtual void init() override
	{
		NESES::Logger::Instance().Init();
		NESESLOG("My app is starting");
	}

	virtual void release() override
	{
		NESESLOG("My app is ending");
		NESES::Logger::Instance().Stop();
	}

};

int main()
{
	std::cout << "hello" << std::endl;
	//std::cout << NESES::StringUtil::NewGuid() << std::endl;

	//NESES::BackObject back;
	//std::string cp1254 = "\xE7\xF0\xFE\xDD";
	//std::string res = NESES::StringUtil::ToUtf8(cp1254,NESES::StringUtil::cEncoding::e_win_1254, back);
	//std::cout << res << std::endl;

	auto myappconfig = std::make_unique<MyApp>();
	NESES::Application::setImpl(std::move(myappconfig));
	NESES::Application::Instance().Init();

	Sleep(200);

	NESES::Application::Instance().ReleaseInstance();

	return 0;
}