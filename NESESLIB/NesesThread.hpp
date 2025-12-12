#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <utility>
#include <functional>
#include "NesesString.hpp"
#include "CallBack.hpp"


namespace NESES
{
	class NesesThread
	{
	private:
		std::string id_;
		std::string name_;
		std::function<void()> callable_{nullptr};
		CallBack<> onStopNotify;
		std::atomic<bool> stopFlag_{ false };
		std::atomic<bool> isDone_{ false };
		std::atomic<bool> isSet_{ false };
		std::atomic<bool> isStarted{ false };
		std::thread th_;


		NesesThread(const std::string& name) : name_(name)
		{
			id_ = StringUtil::NewGuid();
		}

	public:



		~NesesThread()
		{
			Stop();
		}

		// non-copyable
		NesesThread(const NesesThread&) = delete;
		NesesThread& operator =(const NesesThread&) = delete;

		// move constructor: transfer thread and flags
		NesesThread(NesesThread&& other) noexcept
			: name_(other.name_), th_(std::move(other.th_)),
			callable_(other.callable_), 
			stopFlag_(other.stopFlag_.load()), 
			isDone_(other.isDone_.load()), 
			isSet_(other.isSet_.load()),
			isStarted(other.isStarted.load())
		{
			// leave other in a stopped/reset state
			other.stopFlag_.store(false);
			other.isDone_.store(false);
			other.isSet_.store(false);
			other.isStarted.store(false);
			other.callable_ = nullptr;
			other.th_ = std::thread();
		}

		// move assignment: join current thread, then take ownership
		NesesThread& operator=(NesesThread&& other) noexcept
		{
			if (this != &other)
			{
				Stop();//todo stop notifier problem

				// move thread
				th_ = std::move(other.th_);
				// copy flags
				stopFlag_.store(other.stopFlag_.load());
				isDone_.store(other.isDone_.load());
				isSet_.store(other.isSet_.load());
				isStarted.store(other.isStarted.load());

				// reset other
				other.stopFlag_.store(false);
				other.isDone_.store(false);
				other.isSet_.store(false);
				other.callable_ = nullptr;
				other.isStarted.store(false);
				other.th_ = std::thread();

			}
			return *this;
		}

		// Accessors
		const std::string& GetName() const { return name_; }
		const std::string& GetId() const { return id_; }

		bool GetStopFlag() const { return stopFlag_.load(); }
		void SetStopFlag(bool stopflag) 
		{ 
			stopFlag_.store(stopflag); 
			if(stopflag)
				onStopNotify.invoke();
		}

		bool GetIsDone() const { return isDone_.load(); }
		void SetIsDone(bool isdone) { isDone_.store(isdone); }

		template<typename Function, typename... Args>
		void Set(Function&& f, Args&& ... args)
		{
			callable_ = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
			isSet_.store(true);
		}		

		void RegisterNotifierCB(const std::function<void()>& func)
		{
			onStopNotify.setCallback(func);
		}

		bool Start()
		{
			if (isStarted.load()) return false;

			//check callable is set 
			if (!isSet_.load())
			{
				std::cout << "Worker thread failed to start! Callable not set! Name: " << GetName() << " Id: " << GetId() << std::endl;
				return false;
			}
			isDone_.store(false);
			stopFlag_.store(false);

			try
			{
				th_ = std::thread(callable_);
			}
			catch (const std::exception& ex)
			{
				th_ = std::thread();
				std::cout << "Worker thread failed to start! Name: " << GetName() << " Id: " << GetId() + " Ex: " + ex.what() << std::endl;
				return false;
			}
			//isDone_.store(true);
			isStarted.store(true);
			return true;
		}

		void Stop()
		{	

			SetStopFlag(true);
			if (th_.joinable())
			{
				try
				{
					th_.join();
					th_ = std::thread(); // reset thread to default-constructed (non-joinable) state
					isDone_.store(true);
					isStarted.store(false);
				}
				catch (const std::exception& ex)
				{
					std::cout << "Worker thread failed to join! Name: " << GetName() << " Id: " << GetId() + " Ex: " + ex.what() << std::endl;
				}
			}
		}

		template<typename T>
		friend class ThreadManager;
	};
}