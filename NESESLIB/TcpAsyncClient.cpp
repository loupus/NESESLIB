#include "TcpAsyncClient.hpp"
#include "boost/asio.hpp"


struct NESES::TcpAsyncClient::TASCImp
{
	boost::asio::io_context ioc;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::steady_timer connTimer;
	boost::asio::steady_timer writeTimer;
	boost::asio::steady_timer hbTimer;
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;
	boost::asio::streambuf recvBuf;

	TASCImp(const TASCImp&) = delete;
	TASCImp& operator=(const TASCImp&) = delete;

	//todo move ctor and assignment?

	TASCImp()
		:socket_(ioc), 
		strand_(boost::asio::make_strand(ioc)),
		writeTimer(ioc),
		connTimer(ioc),
		hbTimer(ioc),
		work_guard_(boost::asio::make_work_guard(ioc))
	{

	}
	
};

void NESES::TcpAsyncClient::do_read()
{
	if (!pimpl->socket_.is_open())
	{
		cbError.invoke("socket is not avaliable");
		return;
	}

	// if (ioc.stopped())
	//{
	//	cbError.invoke("ioc is not avaliable");
	//	return;
	// }

	if (stopFlag.load())
	{
		cbInfo.invoke("Stop flag detected: top of do_read");
		cbInfo.invoke("exiting read loop");
		return;
	}

	boost::asio::async_read_until(
		pimpl->socket_,
		pimpl->recvBuf,
		readDelim_,
		boost::asio::bind_executor(
			pimpl->strand_,
			[this](boost::system::error_code ec, std::size_t length)
			{
				if (ec)
				{
					cbError.invoke("async read error:" + ec.message());
					isDone = true;
					pimpl->socket_.cancel();
					return;
				}
				else
				{
					size_t dataSize = pimpl->recvBuf.size();
					while (pimpl->recvBuf.size() > 0)
					{
						size_t delimSize = readDelim_.size();
						const char* data = static_cast<const char*>(pimpl->recvBuf.data().data());
						const char* pos = std::search(data, data + dataSize, readDelim_.begin(), readDelim_.end());
						if (pos != data + dataSize)
						{
							size_t messageSize = pos - data;
							std::string message(data, messageSize);
							pimpl->recvBuf.consume(messageSize + delimSize);
							cbReceived.invoke(message, length);
						}
						else
							break;
					}

					if (stopFlag.load())
					{
						cbInfo.invoke("Stop flag detected: bottom of do_read");
						cbInfo.invoke("exiting read loop");
						return;
					}
					do_read();
				}
			}));
}

void NESES::TcpAsyncClient::sendHeartBeat()
{
	if (stopFlag.load())
		return;

	std::string strHb = hbCommand_;
	boost::asio::post(pimpl->strand_, [this, strHb]()
		{ boost::asio::async_write(
			pimpl->socket_,
			boost::asio::buffer(strHb.c_str(), strHb.length()),
			boost::asio::bind_executor(
				pimpl->strand_,
				[this](boost::system::error_code ec, std::size_t bytes_transferred)
				{
					if (ec)
					{
						cbError.invoke("Connection problem!");
						cbConnected.invoke(false);
					}
					else
					{
						if (logHeartbeat)
							cbInfo.invoke("Heartbeat sent");
					}
				})); });
}

void NESES::TcpAsyncClient::HeartBeat()
{
	if (stopFlag.load())
		return;
	pimpl->hbTimer.expires_after(std::chrono::seconds(1));
	pimpl->hbTimer.async_wait(boost::asio::bind_executor(
		pimpl->strand_,
		[this](const boost::system::error_code& ec)
		{
			if (hbStopFlag.load())
				return;

			else if (ec)
			{
				cbError.invoke("heartbeat timer expired");
				return;
			}

			sendHeartBeat();
			HeartBeat(); // reschedule next beat
		}));
}

NESES::TcpAsyncClient::TcpAsyncClient(int conntimeoutsecs, int writetimeoutsecs, int readtimeoutsecs)
	: pimpl(std::make_unique<TASCImp>()),
	IsEndPointOk(false),
	isStarted(false),
	isConnected(false),
	logHeartbeat(true),
	stopFlag(false),
	hbStopFlag(false),
	connTimeoutSecs(conntimeoutsecs),
	writeTimeoutSecs(writetimeoutsecs),
	readTimeoutSecs(readtimeoutsecs)
{
}

NESES::TcpAsyncClient::~TcpAsyncClient()
{
	DisConnect();
}

void NESES::TcpAsyncClient::WaitSeconds(int seconds)
{
	std::promise<void> prom;
	auto fut = prom.get_future();

	boost::asio::steady_timer timer(pimpl->ioc);
	timer.expires_after(std::chrono::seconds(seconds));

	// Run the timer in the io_context
	timer.async_wait(boost::asio::bind_executor(
		pimpl->strand_,
		[&prom](const boost::system::error_code& ec)
		{
			if (!ec)
				prom.set_value(); // unblock the caller
			else
				prom.set_exception(std::make_exception_ptr(std::runtime_error("Timer error")));
		}));

	// Block the caller thread here, not io_context
	fut.get();
}

void NESES::TcpAsyncClient::Connect(BackObject& back)
{
	back.Reset(true);
	if (!IsEndPointOk)
	{
		back.Success = false;
		back.ErrDesc = "Endpoint is not OK! ";
		return;
	}
	pimpl->connTimer.cancel();
	pimpl->connTimer.expires_after(std::chrono::seconds(connTimeoutSecs));
	pimpl->connTimer.async_wait(boost::asio::bind_executor(pimpl->strand_,
		[this](const boost::system::error_code& err)
		{
			if (!err && !isConnected)
			{
				cbError.invoke("socket connect timeout! " + err.message());
				pimpl->socket_.cancel();
			}
		}));

	pimpl->socket_.async_connect(pimpl->endpoint, boost::asio::bind_executor(
		pimpl->strand_,
		[this](const boost::system::error_code ec)
		{
			pimpl->connTimer.cancel();
			if (ec)
			{
				cbError.invoke("socket connect failed! " + ec.message());
				pimpl->socket_.cancel();
			}
			else
			{
				isConnected = true;
				cbConnected.invoke(true);
			}
		}));
}

void NESES::TcpAsyncClient::Connect2(BackObject& back)
{
	back.Reset(true);
	try
	{
		pimpl->socket_.connect(pimpl->endpoint);
	}
	catch (const std::exception&)
	{
		back.Success = false;
		back.ErrDesc = "Connect failed";
	}
}

void NESES::TcpAsyncClient::DisConnect()
{
	Stop();
	if (pimpl->socket_.is_open())
		pimpl->socket_.close();
	if (isConnected)
	{
		isConnected = false;
		cbConnected.invoke(false);
	}
}

void NESES::TcpAsyncClient::Set(const TcpClientContext& cc, BackObject& back)
{
	back.Reset(true);
	if (cc.ipAddress.empty() || cc.port == 0)
	{
		back.Success = false;
		back.ErrDesc = "Endpoint not specified!";
		return;
	}

	if (cc.heartbeatCommand.empty())
	{
		back.Success = false;
		back.ErrDesc = "Heartbeat command not specified!";
		return;
	}

	if (cc.f_receive == nullptr)
	{
		back.Success = false;
		back.ErrDesc = "Receive callback not specified!";
		return;
	}

	if (cc.port < 1025 || cc.port > 65535)
	{
		back.Success = false;
		back.ErrDesc = "Port number must be between 1025 and 65535 !";
		return;
	}

	if (cc.readDelimiter.empty())
	{
		back.Success = false;
		back.ErrDesc = "Read delimiter not specified!";
		return;
	}

	boost::system::error_code ec;
	auto ip = boost::asio::ip::make_address_v4(cc.ipAddress, ec);
	if (ec)
	{
		back.Success = false;
		back.ErrDesc = "Create endpoint failed! " + ec.message();
		return;
	}
	pimpl->endpoint.address(ip);
	pimpl->endpoint.port(cc.port);
	IsEndPointOk = true;
	readDelim_ = cc.readDelimiter;
	hbCommand_ = cc.heartbeatCommand;
	cbInfo.setCallback(cc.f_info);
	cbError.setCallback(cc.f_err);
	cbReceived.setCallback(cc.f_receive);
	cbConnected.setCallback(cc.f_connected);
	logHeartbeat = cc.logheartbeat;
}

bool NESES::TcpAsyncClient::IsOpen() const
{
	return pimpl->socket_.is_open();
}

void NESES::TcpAsyncClient::AsyncWrite(const std::string& strRequest, size_t& sentBytes, BackObject& back)
{
	back.Reset(true);
	if (stopFlag.load())
	{
		back.Success = false;
		back.Warning = "Stop flag detected: async write!";
		return;
	}
	if (!IsOpen() || !isStarted)
	{
		back.Success = false;
		back.ErrDesc = "Socket not connected or client not started";
		return;
	}
	if (strRequest.empty())
	{
		back.Success = false;
		back.ErrDesc = "Empty request";
		return;
	}

	boost::asio::post(
		pimpl->strand_,
		[this, strRequest]()
		{
			boost::asio::async_write(
				pimpl->socket_,
				boost::asio::buffer(strRequest.c_str(), strRequest.length()),
				boost::asio::bind_executor(
					pimpl->strand_,
					[this](boost::system::error_code ec, std::size_t length)
					{
						if (ec)
							cbError.invoke("Write operation failed: " + ec.message());
					}));
		});
}

void NESES::TcpAsyncClient::Start()
{
	if (isStarted)
		return;
	stopFlag = false;
	isDone = false;

	service_thread = std::thread(
		[this]()
		{
			cbInfo.invoke("Starting ioc thread");
			auto rt = pimpl->ioc.run();
			cbInfo.invoke("Exiting ioc thread");
		});
	isStarted = true;
	// std::this_thread::sleep_for(std::chrono::seconds(3));
	do_read();
	HeartBeat();
}
void NESES::TcpAsyncClient::Stop()
{
	if (!isStarted)
		return;

	cbInfo.invoke("stopping io context");
	boost::system::error_code ignored_ec;
	stopFlag.store(true);
	hbStopFlag.store(true);

	// if (socket.is_open())
	//	socket.cancel(ignored_ec);		// Cancels all pending async ops (read/write)
	// will trigger all pending async_read, async_write, etc. with boost::asio::error::operation_aborted.

	pimpl->writeTimer.cancel(); // Unblocks async_wait() if it's pending
	pimpl->connTimer.cancel();
	pimpl->hbTimer.cancel();
	// Allow pending operations to complete
	// ioc.run_one(); // Process any queued handlers
	// ioc.stop();				// Stops further handler invocation
	pimpl->work_guard_.reset();
	pimpl->ioc.stop();
	// cbInfo.invoke("io context stopped");

	pimpl->socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

	if (service_thread.joinable())
		service_thread.join();
	isStarted = false;
	cbInfo.invoke("io context stopped");
}
