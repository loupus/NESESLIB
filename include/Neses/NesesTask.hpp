#pragma once
#include <atomic>
#include <string>
#include <future>
#include <utility>
#include "CallBack.hpp"
#include "NesesString.hpp"

namespace NESES
{

    /**
     * @brief Lightweight wrapper around a callable + packaged_task used by TaskPool.
     *
     * @tparam ReturnType The callable's return type. The internal `TaskType` is `std::packaged_task<ReturnType()>`.
     *
     * @details
     * - Constructed with a name (private); TaskPool is a friend and creates instances.
     * - Call `Set(...)` to bind a callable and initialize the internal packaged_task.
     * - Call `GetFuture()` (only once) to obtain the future associated with the packaged_task.
     * - `operator()()` invokes the packaged_task (if valid).
     * - Use `SetStopFlag(true)` to request cooperative cancellation; the callable should check `GetStopFlag()`.
     */
    template <typename ReturnType>
    class NesesTask
    {
    private:
        /// Underlying task type storing the callable and shared state.
        using TaskType = std::packaged_task<ReturnType()>;

        std::string id_;            /**< Unique id generated at construction. */
        std::atomic<bool> stopflag_;/**< Cooperative stop flag for the task. */
        TaskType task_;             /**< The packaged_task that will run the callable. */
        std::string name_;          /**< Human-readable task name. */

        /**
         * @brief Construct a named NesesTask.
         * @param name Human-readable task name.
         *
         * @note This constructor is private; TaskPool is declared a friend and is expected to create tasks.
         */
        NesesTask(const std::string& name) : stopflag_(false), name_(name)
        {
            id_ = StringUtil::NewGuid();
        }

    public:
        /**
         * @brief Callbacks the caller may attach to observe task progress/completion.
         *
         * - cbCompleted_: called when task completes (user-managed).
         * - cbProgress_: progress callback with int progress, string message (user-managed).
         */
        CallBack<const std::string&> cbCompleted_;
        CallBack<int, const std::string&> cbProgress_;

        /**
         * @brief Bind a callable and its arguments to this task.
         *
         * The callable is bound using std::bind and stored in the internal packaged_task.
         * After calling `Set(...)`, `IsValid()` will return true.
         *
         * @tparam Func Callable type.
         * @tparam Args Argument types forwarded to the callable.
         * @param func Callable or function pointer.
         * @param args Arguments forwarded to the callable when invoked.
         *
         * @note Call `GetFuture()` immediately after `Set(...)` if you need the returned future,
         *       because `get_future()` can be called only once for a given packaged_task.
         */
        template <typename Func, typename... Args>
        void Set(Func&& func, Args&&... args)
        {
            auto boundtask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            task_ = TaskType(boundtask);
        }

        /**
         * @brief Get the future associated with the packaged task.
         * @return std::future<ReturnType> A future holding the task result, or a default-constructed future if not valid.
         *
         * @warning `task_.get_future()` may be called only once for a given packaged_task. Call this before
         *          publishing the task to other threads (before Enqueue).
         */
        std::future<ReturnType> GetFuture()
        {
            if (task_.valid())
                return task_.get_future();
            else
                return std::future<ReturnType>();
        }

        /**
         * @brief Destructor.
         *
         * Requests non-stop by setting stop flag to false (implementation detail).
         * Destructor must not throw.
         */
        ~NesesTask()
        {
            SetStopFlag(false);
        }

        /**
         * @brief Query the cooperative stop flag.
         * @return true if stop requested, false otherwise.
         */
        bool GetStopFlag() const
        {
            return stopflag_.load();
        }

        /**
         * @brief Set or clear the cooperative stop flag for this task.
         * @param stopflag true to request stop, false to clear.
         *
         * @note The callable executed by the task should poll `GetStopFlag()` and return promptly when true.
         */
        void SetStopFlag(bool stopflag)
        {
            stopflag_.store(stopflag);
        }

        /**
         * @brief Invoke the stored packaged_task if valid.
         *
         * The packaged_task is executed synchronously on the calling thread.
         * If the packaged_task is invalid (`IsValid()` == false), nothing is invoked.
         */
        void operator()()
        {
            if (task_.valid())
                task_();

#if _DEBUG
            else
                std::cout << "invalid task" << std::endl;
#endif
        }

        /**
         * @brief Get the task name.
         * @return std::string Task name (copy).
         */
        std::string GetName()
        {
            return name_;
        }

        /**
         * @brief Get the task id.
         * @return const std::string Task GUID (copyable).
         */
        const std::string GetId()
        {
            return id_;
        }

        /**
         * @brief Check whether the underlying packaged_task has been initialized.
         * @return true when a callable is bound (Set was called and the packaged_task is valid).
         *
         * @note `IsValid()` becomes false if the packaged_task was moved-from.
         */
        bool IsValid() const noexcept
        {
            return task_.valid();
        }

        /* Allow TaskPool to construct NesesTask instances and access private members. */
        template<typename T>
        friend class TaskPool;
    };

}