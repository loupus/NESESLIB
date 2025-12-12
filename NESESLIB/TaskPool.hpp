#pragma once
#include <vector>
#include <thread>
#include <deque>
#include <future>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <memory>
#include "NesesTask.hpp"

namespace NESES
{
    /**
     * @brief Simple thread pool + job queue.
     *
     * @tparam ReturnType Task return type (used by NesesTask<ReturnType>).
     *
     * @remarks
     * Producers create or obtain `NesesTask<ReturnType>` objects, call `Enqueue(...)` to submit them,
     * and worker threads created by the pool pop and execute tasks until the pool is stopped.
     *
     * Thread-safety summary:
     * - `workerVectorLock` protects `workers_`.
     * - `taskQueueLock` protects `tasks` and taskCount().
     * - `stopFlag` is atomic and used to signal shutdown.
     */
    template<typename ReturnType>
    class TaskPool
    {
    private:

        size_t maxWorkerCount_;                                      /**< maximum number of worker threads */
        size_t maxTaskCount_;                                        /**< maximum number of queued tasks */
        std::vector<std::thread> workers_;                          /**< storage for running worker threads */
        std::deque<std::shared_ptr<NesesTask<ReturnType>>> tasks;   /**< task queue */
        mutable std::mutex workerVectorLock;                        /**< mutex protecting workers_ */
        mutable std::mutex taskQueueLock;                           /**< mutex protecting tasks */
        std::condition_variable cv;                                 /**< notifies workers of new tasks or shutdown */
        std::atomic<bool> stopFlag{ false };                        /**< pool shutdown flag */

        /**
         * @brief Worker main loop.
         *
         * Waits until either:
         * - `stopFlag` is set (shutdown requested), or
         * - there is at least one task in the queue.
         *
         * When a task is available the worker pops it (under lock) and executes it outside the lock.
         * Exceptions thrown by tasks are caught and logged so the worker can continue processing.
         *
         * Exits when stopFlag is true and the queue is empty.
         */
        void WorkerFunction()
        {
            while (true)
            {
                std::shared_ptr<NesesTask<ReturnType>> task{ nullptr };

                {
                    std::unique_lock<std::mutex> lock(taskQueueLock);
                    cv.wait(lock, [this] {
                        return stopFlag.load() || !tasks.empty();
                        });

                    if (stopFlag.load() && tasks.empty())
                    {
                        return;
                    }

                    if (!tasks.empty())
                    {
                        task = tasks.front();
                        tasks.pop_front();
                    }
                }

                try
                {
                    // Execute the task (defensive null check)
                    if (!task) continue;
                    (*task)();
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Task execution error: " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Unknown error during task execution!" << std::endl;
                }
            }
        }

        /**
         * @brief Create and start a worker thread if below maxWorkerCount_.
         *
         * The new std::thread is emplaced into `workers_`; constructing the std::thread
         * starts execution of `WorkerFunction` immediately.
         *
         * @note Protected by `workerVectorLock` to avoid races on workers_.
         */
        void CreateWorker()
        {
            std::unique_lock<std::mutex> lock(workerVectorLock);
            if (workers_.size() < maxWorkerCount_)
            {
                workers_.emplace_back(&TaskPool::WorkerFunction, this);
            }
        }

    public:

        /**
         * @brief Construct a TaskPool.
         *
         * Sets `maxWorkerCount_` from std::thread::hardware_concurrency() with a fallback to 1,
         * and reserves the worker vector to avoid reallocation.
         */
        TaskPool(const size_t maxtaskcount)
            :maxWorkerCount_(std::thread::hardware_concurrency())
            ,maxTaskCount_(maxtaskcount)
        {
            if (maxWorkerCount_ == 0) maxWorkerCount_ = 1;
            workers_.reserve(maxWorkerCount_);
        }

        /**
         * @brief Destructor: perform graceful shutdown.
         *
         * Calls Stop() to notify and join worker threads.
         */
        ~TaskPool()
        {
            StopAll();
        }

        /**
         * @brief Create an empty named task. Caller must call Set(...) before Enqueue().
         *
         * @param name Human-readable task name.
         * @return shared_ptr to a new NesesTask or nullptr if pool is stopping.
         */
        std::shared_ptr<NesesTask<ReturnType>> GetNew(const std::string& name)
        {
            if (stopFlag.load()) return nullptr;
            return std::shared_ptr<NesesTask<ReturnType>>(new NesesTask<ReturnType>(name));
        }

        /**
         * @brief Create a named task and set its callable immediately.
         *
         * Convenience overload that constructs a task and calls Set(func, args...).
         *
         * @tparam Func Callable type.
         * @tparam Args Argument types forwarded to the callable.
         * @param name Task name.
         * @param func Callable to run when the task is executed.
         * @param args Arguments bound to the callable.
         * @return shared_ptr to configured NesesTask or nullptr if pool is stopping.
         */
        template <typename Func, typename... Args>
        std::shared_ptr<NesesTask<ReturnType>> GetNew(const std::string& name, Func&& func, Args&&... args)
        {
            if (stopFlag.load()) return nullptr;
            auto back = std::shared_ptr<NesesTask<ReturnType>>(new NesesTask<ReturnType>(name));
            back->Set(std::forward<Func>(func), std::forward<Args>(args)...);
            return back;
        }

        /**
         * @brief Enqueue a task for execution.
         *
         * The function verifies:
         * - task is non-null and valid,
         * - pool is not stopping,
         * - queue is not full (maxTaskCount).
         *
         * If accepted, a worker is lazily created (if needed), the task is pushed and one worker is notified.
         *
         * @param task Shared pointer to a configured NesesTask (must be Set()).
         * @return true if task accepted, false otherwise.
         *
         * @note Caller should obtain the task future (GetFuture()) before Enqueue() to avoid races.
         */
        bool Enqueue(std::shared_ptr<NesesTask<ReturnType>> task)
        {
            if (!task || !task->IsValid()) return false;   // defensive check
            if (stopFlag.load()) return false;
            if (taskCount() >= maxTaskCount_) return false;

            CreateWorker();
            {
                std::unique_lock<std::mutex> lock(taskQueueLock);
                tasks.push_back(std::move(task));
            }
            cv.notify_one();
            return true;
        }

        /**
         * @brief Graceful shutdown of the pool.
         *
         * - Sets each queued task's stop flag (cooperative cancellation).
         * - Sets pool stopFlag and notifies all workers.
         * - Joins all worker threads.
         *
         * After Stop returns no worker threads are running.
         */
        void StopAll()
        {
            // stop tasks (request cooperative cancellation)
            {
                std::unique_lock<std::mutex> lock(taskQueueLock);
                for (auto it = tasks.begin(); it != tasks.end(); it++)
                {
                    (*it)->SetStopFlag(true);
                }
            }

            // stop task pool
            stopFlag.store(true);
            cv.notify_all();
            for (std::thread& worker : workers_) {
                if (worker.joinable())
                    worker.join();
            }
        }

        /**
         * @brief Current number of worker threads stored.
         * @return worker count (protected by workerVectorLock).
         */
        size_t workerCount() const
        {
            std::unique_lock<std::mutex> lock(workerVectorLock);
            return workers_.size();
        }

        /**
         * @brief Current queued task count.
         * @return number of tasks in the queue (protected by taskQueueLock).
         */
        size_t taskCount() const
        {
            std::unique_lock<std::mutex> lock(taskQueueLock);
            return tasks.size();
        }

    };
}