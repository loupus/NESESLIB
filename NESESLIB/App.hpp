#pragma once
#include <iostream>
#include <memory>
#include "BackObject.hpp"
#include "ThreadManager.hpp"
#include "TaskPool.hpp"

namespace NESES
{
	constexpr size_t maxthreadcount = 4;
	constexpr size_t maxconcurrenttaskcount = 16;

	
	class AppImplBase 
	{
	public:
		virtual ~AppImplBase() = default;
		virtual void init() = 0;
		virtual void release() = 0;
	};

	class Application 
	{

	private:
		std::unique_ptr<AppImplBase> impl;
		ThreadManager<NesesThread> tm;
		TaskPool<BackObject> tpool;

		Application()
			:tm(maxthreadcount)
			, tpool(maxconcurrenttaskcount)
		{
			//std::cout << "application instance created." << std::endl;
		}
		~Application()
		{
			ReleaseInstance();
			//std::cout << "application instance destroyed." << std::endl;
		}


		Application(const Application&) = delete;
		Application& operator =(const Application&) = delete;

	public:

		static Application& Instance()
		{
			static Application _instance;
			return _instance;
		}

		static void setImpl(std::unique_ptr<AppImplBase> customImpl)
		{
			Instance().impl = std::move(customImpl);
		}

		void ReleaseInstance()
		{
			if (impl)
				impl->release();
			StopWorkers();		
		}

		void Init()
		{
			if (impl)
				impl->init();
		}

		std::shared_ptr<NesesThread> NewWorker(const std::string& name = "")
		{
			return tm.GetNew(name);
		}

		std::shared_ptr<NesesTask<BackObject>> NewTask(const std::string& name = "")
		{
			return tpool.GetNew(name);
		}

		// task starts when you enqueue, dont forget to set and get future before enqueuing if you need
		bool EnqueueTask(std::shared_ptr<NesesTask<BackObject>> task)
		{
			return tpool.Enqueue(task);
		}

		void StopWorkers()
		{
			tm.StopAll();
		}

		void StopTasks()
		{
			tpool.StopAll();
		}
	};

	using App = Application;
}// namespace NESES


