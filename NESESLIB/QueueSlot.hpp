#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include <cassert>
#include <array>

namespace NESES
{
template<typename T, int RangeN>
class LockFreeSignedSlotArray 
{
    static_assert(RangeN > 0, "RangeN must be positive");

public:
    using value_type = T;
    static constexpr int kMinKey = 0;
    static constexpr int kMaxKey = RangeN;
    static constexpr size_t kArraySize = RangeN + 1;

    LockFreeSignedSlotArray() 
    {
        for (auto& slot : slots_) {
            slot.store(nullptr, std::memory_order_relaxed);
        }
    }

    bool set(int key, T* value)
    {
        if (!(is_valid_key(key))) return false;
        if (value == nullptr) return false;
        slots_[key_to_index(key)].store(value, std::memory_order_release);
        return true;
    }

    // peek + clear
    std::optional<T*> try_consume(int key) 
    {
        if (!(is_valid_key(key))) return std::nullopt;
        auto& atomic_slot = slots_[key_to_index(key)];
        T* expected = atomic_slot.load(std::memory_order_acquire);
        if (expected && atomic_slot.compare_exchange_strong(expected, nullptr)) {
            return expected;
        }
        return std::nullopt;
    }

    std::optional<T*> peek(int key) const 
    {
        if (!(is_valid_key(key))) return std::nullopt;
        T* ptr = slots_[key_to_index(key)].load(std::memory_order_acquire);
        return ptr ? std::optional<T*>{ptr} : std::nullopt;
    }

    void clear(int key) 
    {
        if (!(is_valid_key(key))) return;
        slots_[key_to_index(key)].store(nullptr, std::memory_order_release);
    }

private:
    static constexpr bool is_valid_key(int key) 
    {
        return key >= kMinKey && key <= kMaxKey;
    }

    static constexpr size_t key_to_index(int key) 
    {
       // return static_cast<size_t>(key + RangeN);
        return static_cast<size_t>(key);
    }

    std::array<std::atomic<T*>, kArraySize> slots_;
};

}