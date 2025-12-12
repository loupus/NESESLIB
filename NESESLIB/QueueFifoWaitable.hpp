// QueueFifoWaitable.hpp
#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <chrono>
#include <cstddef>

namespace NESES
{

template <typename T>
class QueueFifoWaitable
{
public:
    explicit QueueFifoWaitable(std::size_t capacity)
        : capacity_(capacity), closed_(false)
    {
    }

    ~QueueFifoWaitable()
    {
        close();
    }

    // non-copyable
    QueueFifoWaitable(const QueueFifoWaitable&) = delete;
    QueueFifoWaitable& operator=(const QueueFifoWaitable&) = delete;

    // push by const-ref, returns false if queue is full or closed
    bool push(const T& item)
    {
        {
            std::lock_guard<std::mutex> lg(mutex_);
            if (closed_ || queue_.size() >= capacity_) return false;
            queue_.push_back(item);
        }
        cv_.notify_one();
        return true;
    }

    // push by rvalue, returns false if queue is full or closed
    bool push(T&& item)
    {
        {
            std::lock_guard<std::mutex> lg(mutex_);
            if (closed_ || queue_.size() >= capacity_) return false;
            queue_.push_back(std::move(item));
        }
        cv_.notify_one();
        return true;
    }

    // non-blocking pop: returns false if empty
    bool pop(T& out)
    {
        std::lock_guard<std::mutex> lg(mutex_);
        if (queue_.empty()) return false;
        out = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    // blocking pop: waits until an element is available or the queue is closed.
    // Returns true if an element was popped, false if queue closed and empty.
    bool wait_pop(T& out)
    {
        std::unique_lock<std::mutex> ul(mutex_);
        cv_.wait(ul, [this] { return !queue_.empty() || closed_; });
        if (queue_.empty()) return false; // closed_ && empty
        out = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    // timed wait pop: returns true if popped, false if timeout or closed+empty
    template <class Rep, class Period>
    bool wait_pop_for(T& out, const std::chrono::duration<Rep, Period>& rel_time)
    {
        std::unique_lock<std::mutex> ul(mutex_);
        if (!cv_.wait_for(ul, rel_time, [this] { return !queue_.empty() || closed_; }))
            return false; // timeout
        if (queue_.empty()) return false; // closed_ && empty
        out = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    // mark the queue closed and wake all waiters.
    void close()
    {
        {
            std::lock_guard<std::mutex> lg(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }

    bool is_closed() const
    {
        std::lock_guard<std::mutex> lg(mutex_);
        return closed_;
    }

    bool is_empty() const
    {
        std::lock_guard<std::mutex> lg(mutex_);
        return queue_.empty();
    }

    bool is_full() const
    {
        std::lock_guard<std::mutex> lg(mutex_);
        return queue_.size() >= capacity_;
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lg(mutex_);
        return queue_.size();
    }

    std::size_t capacity() const noexcept { return capacity_; }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<T> queue_;
    const std::size_t capacity_;
    bool closed_;
};

} // namespace NESES