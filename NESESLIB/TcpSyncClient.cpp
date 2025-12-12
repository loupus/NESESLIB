#include "TcpSyncClient.hpp"
#include "boost/asio.hpp"

struct NESES::TcpSyncClient::TSCImp
{
	boost::asio::io_context ioc;
	boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::tcp::socket socket;
	boost::asio::steady_timer timer;
	boost::system::error_code ec;

	TSCImp(const TSCImp&) = delete;
	TSCImp& operator=(const TSCImp&) = delete;

	//todo move ctor and assignment?

	TSCImp()
		:socket(ioc), timer(ioc)
	{

	}

	size_t ExtractSingleResponse(boost::asio::streambuf& sb, std::string& response, const std::string& delim)
	{
		size_t delimSize = delim.size();
		size_t buffSize = sb.size();

		// Early exit if buffer is empty or delimiter is empty
		if (buffSize == 0 || delimSize == 0)
		{
			return buffSize;
		}

		const char* data = static_cast<const char*>(sb.data().data());

		// Search for the delimiter in the data
		const char* pos = std::search(data, data + buffSize, delim.begin(), delim.end());

		if (pos != data + buffSize)
		{									 // Delimiter found
			size_t messageSize = pos - data; // Length of the message before the delimiter

			// Extract the message as a string (excluding the delimiter)
			std::string message(data, messageSize);
			response.append(message);

			// Consume the message along with the delimiter from the buffer
			sb.consume(messageSize + delimSize);
		}
		return sb.size();
	}
};

NESES::TcpSyncClient::TcpSyncClient(int conntimeoutsecs, int writetimeoutsecs, int readtimeoutsecs)
	:pimpl(std::make_unique<TSCImp>()),
	IsEndPointOk(false), 
	connTimeoutSecs(conntimeoutsecs),
	writeTimeoutSecs(writetimeoutsecs),
	readTimeoutSecs(readtimeoutsecs)
{
}

NESES::TcpSyncClient::~TcpSyncClient()
{
}

void NESES::TcpSyncClient::Set(const TcpClientContext& cc, BackObject& back)
{
	back.Reset(true);
	if (cc.ipAddress.empty() || cc.port == 0)
	{
		back.Success = false;
		back.ErrDesc = "Endpoint not specified!";
		return;
	}
	ipaddr_ = cc.ipAddress;

	if (cc.port < 1025 || cc.port > 65535)
	{
		back.Success = false;
		back.ErrDesc = "Port number must be between 1025 and 65535 !";
		return;
	}
	port_ = cc.port;

	if (cc.readDelimiter.empty())
	{
		back.Success = false;
		back.ErrDesc = "Read delimiter not specified!";
		return;
	}
	readDelim_ = cc.readDelimiter;

	boost::system::error_code ec;
	auto ip = boost::asio::ip::make_address_v4(ipaddr_, ec);
	if (ec)
	{
		back.Success = false;
		back.ErrDesc = "Create endpoint failed! " + ec.message();
		return;
	}
	pimpl->endpoint.address(ip);
	pimpl->endpoint.port(port_);
	IsEndPointOk = true;
	readDelim_ = readDelim_;
}

void NESES::TcpSyncClient::Connect(BackObject& back)
{
	back.Reset(true);
	if (!IsEndPointOk)
	{
		back.Success = false;
		back.ErrDesc = "Endpoint not set!";
		return;
	}
	pimpl->ec.clear();

	if (IsOpen())
	{
		back.Warning = "Socket is avaliable!";
		return;
	}

	pimpl->timer.expires_after(std::chrono::seconds(connTimeoutSecs));
	pimpl->timer.async_wait(
		[this](const boost::system::error_code& error)
		{
			if (!error)
			{
				pimpl->socket.cancel(); // Cancel socket operations if timer expires
			}
		});

	pimpl->socket.connect(pimpl->endpoint, pimpl->ec);
	pimpl->timer.cancel();
	if (pimpl->ec)
	{
		back.Success = false;
		back.ErrDesc = "Socket connect failed! " + pimpl->ec.message();
		return;
	}
}

void NESES::TcpSyncClient::DisConnect()
{
	pimpl->timer.cancel();
	if (pimpl->socket.is_open())
	{
		boost::system::error_code ec_ignore;
		pimpl->socket.close(ec_ignore);
	}
}

size_t NESES::TcpSyncClient::Send(const std::string& strRequest, std::string& strResponse, BackObject& back)
{
	back.Reset(true);
	strResponse.clear();

	size_t bytesSent = 0;
	size_t bytesReceived = 0;
	size_t buffSize = 0;
	size_t delimSize = readDelim_.size();
	boost::asio::streambuf recvBuf;
	std::size_t bytes_readable = 0;

	if (strRequest.empty())
	{
		back.Success = false;
		back.ErrDesc = "Empty request";
		return bytesReceived;
	}

	const std::lock_guard<std::mutex> lock(mtx_);

	if (!IsOpen())
	{
		Connect(back);
		if (!back.Success)
			return bytesReceived;
	}

	pimpl->ec.clear();
	pimpl->timer.expires_after(std::chrono::seconds(writeTimeoutSecs));
	pimpl->timer.async_wait(
		[this](const boost::system::error_code& error)
		{
			if (!error)
			{
				std::cout << "write timeout" << std::endl;
				pimpl->socket.cancel();
			}
		});

	bytesSent = boost::asio::write(socket, boost::asio::buffer(strRequest, strRequest.length()), ec);
	pimpl->timer.cancel();
	if (ec)
	{
		back.Success = false;
		back.ErrDesc = "boost::asio::write failed! " + ec.message();
		return bytesReceived;
	}

	pimpl->timer.expires_after(std::chrono::seconds(readTimeoutSecs));
	pimpl->timer.async_wait(
		[this](const boost::system::error_code& error)
		{
			if (!error)
			{
				std::cout << "read timeout" << std::endl;
				pimpl->socket.cancel();
			}
		});

	do
	{
		bytesReceived += boost::asio::read_until(socket, recvBuf, readDelim_, ec);
		pimpl->timer.cancel();
		if (ec)
		{
			back.Success = false;
			if (ec == boost::asio::error::eof)
				back.ErrDesc = "boost::asio::read eof! " + ec.message();
			else
				back.ErrDesc = "boost::asio::read failed! " + ec.message();
			return bytesReceived;
		}

		buffSize = recvBuf.size();
		if (buffSize == 0 || delimSize == 0)
		{
			return bytesReceived;
		}

		buffSize = pimpl->ExtractSingleResponse(recvBuf, strResponse, readDelim_);
		bytes_readable = pimpl->socket.available(ec);
	} while (buffSize > 0 || bytes_readable > 0);
	return bytesReceived;
}

bool NESES::TcpSyncClient::IsOpen() const
{
	return pimpl->socket.is_open();
}
