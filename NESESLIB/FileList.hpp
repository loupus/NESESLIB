#pragma once
#include <mutex>
#include <list>
#include <optional>
#include "FileInfo.hpp"
#include "BackObject.hpp"

namespace NESES
{
	class FileList
	{
	private:
		std::list<FileInfo> q;
		std::mutex mtx;
		size_t BufCapacity{ 10000 };

	public:

		BackObject AddItem(const FileInfo& anItem)
		{
			BackObject back;
			if (q.size() < BufCapacity && !Contains(anItem))
			{
				std::unique_lock<std::mutex> lock(mtx);
				q.push_back(anItem);
				lock.unlock();
				back.Success = true;
			}
			else
			{
				back.ErrDesc = "Already exists or buffer limit reached";
				back.Success = false;
			}
			return back;
		}

		bool Contains(const FileInfo& anItem)
		{
			if (anItem.hash == 0) return false;
			std::unique_lock<std::mutex> lock(mtx);
			auto it = std::find_if(q.begin(), q.end(), [&](const FileInfo& item) {return  (item.IsDeleted == false) && (item.hash == anItem.hash); });
			if (it == q.end())
				return false;
			else
				return true;
		}

		bool RemoveItem(const FileInfo& anItem)
		{
			if (anItem.hash == 0) return false;
			std::unique_lock<std::mutex> lock(mtx);
			auto it = std::find_if(q.begin(), q.end(), [&](const FileInfo& item) {return item.hash == anItem.hash; });
			if (it != q.end())
			{
				q.erase(it);
				return true;
			}
			return false;
		}

		void RemoveDeleted()
		{
			std::unique_lock<std::mutex> lock(mtx);
			q.remove_if([=](FileInfo a) {return a.IsDeleted == true; });
			lock.unlock();
		}

		std::list<FileInfo>::iterator Begin()
		{
			return q.begin();
		}

		std::list<FileInfo>::iterator End()
		{
			return q.end();
		}

		const std::list<FileInfo> GetQ()
		{
			return q;
		}

		FileInfo& Front()
		{
			return q.front();
		}

		void PopFront()
		{
			q.pop_front();
		}

		FileInfo* GetIfContains(const FileInfo& anItem)
		{
			if (anItem.hash == 0)  return nullptr;
			auto it = std::find_if(q.begin(), q.end(), [&](const FileInfo& item) {return (item.IsDeleted == false) && (item.hash == anItem.hash); });
			if (it == q.end())
				return nullptr;
			else
				return &(*it);
		}


		int GetSize()
		{
			return static_cast<int>(q.size());
		}

		void Clear()
		{
			q.clear();
		}
	};

}
