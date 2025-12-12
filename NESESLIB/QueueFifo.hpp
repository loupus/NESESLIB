#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <mutex>

namespace NESES
{

template <typename T>
class FifoQueue
{
private:
	std::queue<T> q_;
	std::mutex queue_lock_;
	const size_t queue_size_;

public:
	FifoQueue(size_t queuesize)
		: queue_size_(queuesize)
	{

	}
	~FifoQueue()
	{

	}

	bool push(const T& item)
	{
		if (is_full()) return false;
		std::lock_guard<std::mutex> lock(queue_lock_);
		q_.push(item);
		return true
	}

	bool push(T&& item)
	{
		if (is_full()) return false;
		std::lock_guard<std::mutex> lock(queue_lock_);		
		q_.emplace(item);
		return true;
	}

	bool pop(T& item)
	{
		if (is_empty()) return false;
		std::lock_guard<std::mutex> lock(queue_lock_);
		item = q_.front();
		q_.pop();
		return true;
	}

	bool is_empty()
	{
		std::lock_guard<std::mutex> lock(queue_lock_);
		return q_.empty();
	}

	bool is_full()
	{		
		return size() == queue_size_;
	}

	const size_t size()
	{
		std::lock_guard<std::mutex> lock(queue_lock_);
		return q_.size();
	}
};
}