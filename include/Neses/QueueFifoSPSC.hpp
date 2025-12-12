#pragma once
#include <atomic>
#include <optional>


// single producer/consumer lockfree circular fifo


/*
Producer (push())
Reads tail_ to check if there�s space:
Uses std::memory_order_acquire to ensure the producer sees all modifications made by the consumer before it updated tail_.
Writes to buffer_[head_] � safe, no sync needed (producer owns it).
Stores head_ with std::memory_order_release:
Ensures that the write to the buffer (the data push) happens-before the consumer sees the updated head_.

Consumer (pop())
Reads head_ to check if there�s data:
Uses std::memory_order_acquire to ensure it sees the data written to the buffer before the producer updated head_.
Reads from buffer_[tail_] � safe, no sync needed (consumer owns it).
Stores tail_ with std::memory_order_release:
Ensures the read is fully completed before the producer sees the updated tail_.

*/
namespace NESES
{

template<typename T, int Capacity>
class SPSCFifoQueue {
public:
    SPSCFifoQueue()
        : head_(0), tail_(0)
    {
    }

    bool push(const T& item) // modifier of head
    {
        auto head = head_.load(std::memory_order_relaxed);
        auto next_head = increment(head);
        if (next_head == tail_.load(std::memory_order_acquire))
        {
            return false; // Queue full
        }

        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool push(T&& item)  // push move
    {
        auto head = head_.load(std::memory_order_relaxed);
        auto next_head = increment(head);
        if (next_head == tail_.load(std::memory_order_acquire))
        {
            return false; // Queue full
        }

        buffer_[head] = std::move(item);
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {  // modififer of tail
        auto tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire))
        {
            return std::nullopt; // Queue empty
        }

        T item = buffer_[tail];
        tail_.store(increment(tail), std::memory_order_release);
        return item;
    }

    bool empty() const
    {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    bool full() const {
        return increment(head_.load(std::memory_order_acquire)) == tail_.load(std::memory_order_acquire);
    }

private:

    static constexpr int BufferSize = Capacity + 1;

    int increment(int i) const
    {
        return (i + 1) % BufferSize;
    }

    std::array<T, BufferSize> buffer_;
    std::atomic<int> head_;
    std::atomic<int> tail_;
};

}
