#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <type_traits>
#include <mutex>
#include "NesesThread.hpp"

/*
todo temizlik işi ve max thread count config
*/

namespace NESES
{

    /* Type Traits: detect Stop/SetStopFlag presence on T (used previously) */
    template<typename T, typename = std::void_t<>>
    struct HasStop : std::false_type {};

    template<typename T>
    struct HasStop<T, std::void_t<decltype(std::declval<T>().Stop())>> : std::true_type {};

    template<typename T, typename = std::void_t<>>
    struct HasSetStopFlag : std::false_type {};

    template <typename T>
    struct HasSetStopFlag<T, std::void_t<decltype(std::declval<T>().SetStopFlag(std::declval<bool>()))>> : std::true_type {};

    template <typename T>
    struct IsStopable_v
    {
        static constexpr bool value = HasStop<T>::value && HasSetStopFlag<T>::value;
    };


    template <typename T>
    class ThreadManager
    {

    private:
        std::mutex tm_mtx;
        std::vector<std::shared_ptr<T>> workers;
       const size_t MaxWorkerCount{ 4 };

    public:
        ThreadManager(const size_t maxworkercount)
          :  MaxWorkerCount(maxworkercount)
        {
            static_assert(IsStopable_v<T>::value, "Type T must have member functions named 'Stop()' and 'SetStopFlag'");
        }
        ~ThreadManager()
        {
            StopAll();
            std::cout << "ThreadManager instance destroyed." << std::endl;
        }

       
        ThreadManager(const ThreadManager&) = delete;
        ThreadManager& operator =(const ThreadManager&) = delete;

        void StopAll()
        {
            std::vector<std::shared_ptr<T>> to_stop;
            {
                std::lock_guard<std::mutex> lock(tm_mtx);
                // request cooperative stop
                for (auto& w : workers) if (w) w->SetStopFlag(true);
                // take ownership of worker ptrs to call Stop() outside the lock
                to_stop = workers;
                workers.clear();
            }

            // call Stop() outside lock; protect against exceptions
            for (auto& w : to_stop)
            {
                if (!w) continue;
                try { w->Stop(); }
                catch (...)
                {
                    // log or swallow — do NOT propagate from StopAll() if it can be used from dtor
                }
            }
        }

        std::size_t Count()  
        {
            std::lock_guard<std::mutex> lock(tm_mtx);
            return workers.size();
        }

        std::shared_ptr<T> GetNew(const std::string& name = "")
        {
            std::lock_guard<std::mutex> lock(tm_mtx);
            if (workers.size() < MaxWorkerCount)
            {
                auto back = std::shared_ptr<T>(new T(name));
                workers.emplace_back(back);
                return back;
            }
            return nullptr;         
        }

        void RemoveFinished()
        {
            std::vector<std::shared_ptr<T>> finished;
            {
                std::lock_guard<std::mutex> lock(tm_mtx);
                for (auto it = workers.begin(); it != workers.end(); )
                {
                    auto& w = *it;
                    if (!w)
                    {
                        it = workers.erase(it);
                        continue;
                    }

                    if (w->GetIsDone())
                    {
                        finished.emplace_back(w);
                        it = workers.erase(it);
                        continue;
                    }
                    ++it;
                }
            }

            // Stop/join finished workers outside the lock
            for (auto& w : finished)
            {
                if (!w) continue;
                try { w->Stop(); }
                catch (...) { /* swallow or log */ }
            }
        }
    };
}