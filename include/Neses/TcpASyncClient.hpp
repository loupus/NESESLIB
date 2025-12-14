#pragma once
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "TcpContext.hpp"
#include "BackObject.hpp"
#include "CallBack.hpp"
#include "Exporter.h"

namespace NESES
{

	class NESESAPI TcpAsyncClient
	{
	private:
		struct TASCImp;
		std::unique_ptr<TASCImp> pimpl;

		std::string readDelim_;
		std::string hbCommand_;
		bool IsEndPointOk{ false };
		bool isStarted{ false };
		bool isConnected{ false };
		bool logHeartbeat{ false };
		const int connTimeoutSecs;
		const int writeTimeoutSecs;
		const int readTimeoutSecs;

		std::thread service_thread;
		std::atomic<bool> stopFlag;
		std::atomic<bool> hbStopFlag;
		std::atomic<bool> isDone;
		std::condition_variable cvStop;
		std::mutex stopLock;

		CallBack<const std::string, size_t> cbReceived;
		CallBack<const std::string> cbInfo;
		CallBack<const std::string> cbError;
		CallBack<bool> cbConnected;

		void do_read();
		void sendHeartBeat();
		void HeartBeat();

	public:
		TcpAsyncClient(const TcpAsyncClient&) = delete;
		TcpAsyncClient& operator=(const TcpAsyncClient&) = delete;

		TcpAsyncClient(int conntimeoutsecs = 3, int writetimeoutsecs = 3, int readtimeoutsecs = 3);
		~TcpAsyncClient();

		void WaitSeconds(int seconds);
		void Connect(BackObject& back);
		void Connect2(BackObject& back);
		void DisConnect();
		void Set(const TcpClientContext& cc, BackObject& back);
		bool IsOpen() const;
		void AsyncWrite(const std::string& strRequest, size_t& sentBytes, BackObject& back);
		void Start();
		void Stop();
	
	};
}
