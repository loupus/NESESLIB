#pragma once
#include <string>
#include <functional>

namespace NESES
{
	struct TcpClientContext
	{
		std::string ipAddress;
		int port;
		std::string readDelimiter;
		std::string heartbeatCommand;
		bool logheartbeat;
		std::function<void(const std::string)> f_info;
		std::function<void(const std::string)> f_err;
		std::function<void(const std::string, size_t)> f_receive;
		std::function<void(bool)> f_connected;
	};

}