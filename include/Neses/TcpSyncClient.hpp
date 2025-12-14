#pragma once
#include <memory>
#include <string>
#include <mutex>
#include "TcpContext.hpp"
#include "BackObject.hpp"
#include "Exporter.h"

namespace NESES
{
	class NESESAPI TcpSyncClient
	{
	private:
		struct TSCImp;
		std::unique_ptr<TSCImp> pimpl;
		std::string ipaddr_;
		std::string readDelim_;
		std::mutex mtx_;
		int port_ = -1;
		bool IsEndPointOk{false};
		const int connTimeoutSecs;
		const int writeTimeoutSecs;
		const int readTimeoutSecs;

	public:
		TcpSyncClient(const TcpSyncClient&) = delete;
		TcpSyncClient& operator=(const TcpSyncClient&) = delete;

		TcpSyncClient(int conntimeoutsecs = 3, int writetimeoutsecs = 3, int readtimeoutsecs=3);
		~TcpSyncClient();

		void Set(const TcpClientContext& cc, BackObject& back);
		void Connect(BackObject& back);
		void DisConnect();
		size_t Send(const std::string& strRequest, std::string& strResponse, BackObject& back);
		bool IsOpen() const;
	};
}


