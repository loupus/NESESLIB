#pragma once
#include <iostream>
#include <fstream>	    // std::ofstream
#include <sstream>		// std::ostringstream
#include <string>
#include <chrono>
#include <ctime>		// std::strftime, std::time_t, std::tm, std::localtime (not thread safe)
#include <time.h>		// localtime_r (windows) localtime_s (posix)
#include <condition_variable>
#include "NesesThread.hpp"
#include "QueueFifoWaitable.hpp"
#include "CallBack.hpp"
#include "App.hpp"					// todo !!! circular header include with app


// developer log macros
#define NESESDLOG(msg) \
    NESES::Logger::Instance().log(__FILE__, __func__, __LINE__, msg, NESES::LogType::info)

#define NESESDLOG_ERROR(msg) \
    NESES::Logger::Instance().log(__FILE__, __func__, __LINE__, msg, NESES::LogType::error)

#define NESESDLOG_WARN(msg) \
    NESES::Logger::Instance().log(__FILE__, __func__, __LINE__, msg, NESES::LogType::warning)


// standard log macros
#define NESESLOG(msg) \
    NESES::Logger::Instance().log(msg, NESES::LogType::info)

#define NESESLOG_ERROR(msg) \
    NESES::Logger::Instance().log(msg, LogType::error)

#define NESESLOG_WARN(msg) \
    NESES::Logger::Instance().log(msg, LogType::warning)

#define NESESLOG_UEVENT(msg) \
    NESES::Logger::Instance().log(msg, LogType::userevent)

// log stream macro
#define NESESLOG_STREAM(lt) \
    NESES::Logger::Instance().log(lt)

namespace NESES
{

	constexpr int MaxLogQueueSize = 255;

	enum class LogType
	{
		info,
		error,
		warning,
		userevent
	};

	
	// todo flush logs yok mu
	class Logger
	{
	private:
		std::string logPath_;
		std::string logTimeFormat_{ "[%Y-%m-%d %H:%M:%S]" };
		std::string logFileFormat_{ "%Y-%m-%d" };
		std::string logFile_;
		std::string currentFile_;
		bool sinkConsole{ true };
		bool sinkFile{ true };
		std::ofstream  ofs;
		bool isStarted;
		std::shared_ptr<NesesThread> consumerTh_;
		CallBack<const std::string&> cbLog_;
		QueueFifoWaitable<std::string> logQueue_;
		std::function<void(std::string)> logHandler_;

		std::string timeStr(const std::string& strFormat)
		{
			auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm tm_buf;
#ifdef _WIN32
			localtime_s(&tm_buf, &now);
#else
			localtime_r(&now, &tm_buf);
#endif
			char timeStr[22] = { 0 };
			std::strftime(timeStr, sizeof(timeStr), strFormat.c_str(), &tm_buf);
			return std::string(timeStr);
		}

		std::string logTypeStr(LogType lt)
		{
			std::string back;
			switch (lt)
			{
			case LogType::error:
			{
				back = "ERROR";
				break;
			}
			case LogType::warning:
			{
				back = "WARNING";
				break;
			}
			case LogType::userevent:
			{
				back = "USEREVENT";
				break;
			}
			case LogType::info:
			default:
			{
				back = "INFO";
				break;
			}
			}
			return back;
		}

		bool AddLog_(const std::string& log)
		{
			bool back = false;				
			back = logQueue_.push(log);	
			return back;

		}

		bool HandleFile(const std::string& newpath = "")
		{
			//new file
			if (currentFile_ != newpath)
			{
				CloseFile();
				if (newpath.empty()) return false;

				try
				{
					ofs.open(newpath.c_str(), std::ios::out | std::ios::app);
					currentFile_ = newpath;
				}
				catch (std::ofstream::failure& e)
				{
					std::string strerr(e.what());
					std::cerr << "Exception opening logfile : " + strerr << std::endl;
					return false;
				}
				return true;
			}


			if (!ofs.is_open())
			{
				try
				{
					ofs.open(logFile_.c_str(), std::ios::out | std::ios::app);
				}
				catch (std::ofstream::failure& e)
				{
					CloseFile();
					std::string strerr(e.what());
					std::cerr << "Exception opening logfile : " + strerr << std::endl;
					return false;
				}
				return true;
			}

			return true;
		}

		void CloseFile()
		{
			ofs.flush();
			ofs.close();
		}

		void WriteLog(std::string log)
		{
			if (!log.empty() && log.back() == '\n')
				log.pop_back();

			if (sinkConsole)
			{
				std::cout << log << std::endl;
			}


			if (sinkFile)
			{
				logFile_.clear();
				logFile_ = logPath_ + timeStr(logFileFormat_) + ".txt";

				if (!HandleFile(logFile_)) return;

				try
				{
					ofs << log << std::endl;
				}
				catch (const std::exception&)
				{
					ofs.close();
					std::cout << "Cannot output log string" << std::endl;
				}

			}
		}

		void OnStopFlag()
		{
			//std::cout << "Logger on stop flag" << std::endl;
			logQueue_.close();
		}

		void ConsumeLogs()
		{
			WriteLog("=====================================================================");

			std::string strlog;
			while (logQueue_.wait_pop(strlog))
			{
				// decide whether there is a new item or time to stop
				if (consumerTh_->GetStopFlag() || logQueue_.is_closed()) break;

				if (!strlog.empty())
				{
					if (logHandler_)
					{
						logHandler_(strlog);
					}
					else
					{
						WriteLog(strlog);
					}						
					cbLog_.invoke(strlog);
				}
				strlog.clear();

				if (consumerTh_->GetStopFlag() || logQueue_.is_closed()) break;

			}


				/*if (consumerTh_->GetStopFlag() && logQueue_.is_empty())
					break;*/
			
		}

		Logger()
			:isStarted(false),
			logPath_("."),
			logQueue_(MaxLogQueueSize)
		{
		}

		public:
			Logger(Logger&) = delete;
			Logger& operator=(const Logger&) = delete;
			Logger(Logger&&) = delete;
			Logger& operator=(Logger&&) = delete;

			~Logger()
			{
				Stop();
			}

			// --- Singleton: Static accessor ---
			static Logger& Instance()
			{
				static Logger instance;
				return instance;
			}

			void Init(				
				std::function<void(std::string)> handler = nullptr, 
				const std::string logpath = ".",
				bool defHandlerSinkConsole = true,
				bool defHandlerSinkFile = true,
				const std::string defHandlerTimeFormat =  "[%Y-%m-%d %H:%M:%S]",
				const std::string defHandlerFileFormat =  "%Y-%m-%d"
				)
			{
				logPath_ = logpath;
				logHandler_ = handler;
				sinkConsole = defHandlerSinkConsole;
				sinkFile = defHandlerSinkFile;
				logTimeFormat_ = defHandlerTimeFormat;
				logFileFormat_ = defHandlerFileFormat;
				consumerTh_ = App::Instance().NewWorker("logger");
				consumerTh_->RegisterNotifierCB(std::bind(&Logger::OnStopFlag, this));
				Start();
			}

			void Stop()
			{
				if (!isStarted) return;
				//consumerTh_->SetStopFlag(true);
				consumerTh_->Stop();
				isStarted = false;
				CloseFile();
				std::cout << "Quiting Logger" << std::endl;
			}

			void Start()
			{
				if (isStarted) return;
				std::cout << "Starting Logger" << std::endl;
				consumerTh_->Set(&Logger::ConsumeLogs, this);
				consumerTh_->SetStopFlag(false);
				consumerTh_->Start();
				isStarted = true;
			}


			// logstream
				// Stream class for building log messages
			class LogStream {
			public:
				LogStream(Logger& logger, LogType level) : logger_(logger), level_(level) {}
				~LogStream()
				{
					flush();
				}

				// Overload << for various types
				template <typename T>
				LogStream& operator<<(const T& value)
				{
					stream_ << value;
					return *this;
				}

				// Non-template overloads are preferred over template instantiations
				// Handle manipulators (e.g., std::endl, '\n')
				LogStream& operator<<(std::ostream& (*manip)(std::ostream&))
				{
					stream_ << manip;
					if (manip == static_cast<std::ostream & (*)(std::ostream&)>(std::endl) ||
						manip == static_cast<std::ostream & (*)(std::ostream&)>(std::flush)) {
						flush();
					}
					return *this;
				}

			private:
				void flush()
				{
					std::string message = stream_.str();
					if (!message.empty())
					{
						logger_.log(message, level_);
					}
					stream_.str(""); // Clear stream
				}

				Logger& logger_;
				LogType level_;
				std::ostringstream stream_;
			};


			// accessors

			void log(const std::string msg, LogType lt = LogType::info)
			{
				std::string strLog = timeStr(logTimeFormat_) + " : " + logTypeStr(lt) + " : " + msg;
				if (!AddLog_(strLog))
					std::cout << "Logger unavaliable " << std::endl;
			}

			void log(const char* codefile, const char* funcname, int linenumber, const std::string& msg, LogType lt = LogType::info)
			{
				std::string strLog = 
					timeStr(logTimeFormat_) + " : " 
					+ logTypeStr(lt) + " : "
					+ codefile + " : " 
					+ funcname + " : " 
					+ std::to_string(linenumber) + " : " 
					+ msg;
				if (!AddLog_(strLog))
					std::cout << "Log unavaliable " << std::endl;
			}

			LogStream log(LogType logtype)
			{
				return LogStream(*this, logtype);
			}
	};

}