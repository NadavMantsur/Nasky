/*******************************************************************************
Project:    Thread Pool
Author:     HRD28
Version:    1.0 - 06/06/2023
            1.1 - 07/06/2023 - changed ITask Do()'s return value to void
            1.2 - 10/06/2023 - untimed Stop() return value is void
            1.3 - 11/06/2023 - switched from unique_ptr to shared_ptr
            1.4 - 11/06/2023 - added virtual Dtor in PriorityImp for inheritence
                               by inner implementation tasks
            1.5 - 12/06/2023 - ITask inherite protected virtual PriorityImp
                               instead of private
*******************************************************************************/
#ifndef __ILRD_HRD28_THREAD_POOL_HPP__
#define __ILRD_HRD28_THREAD_POOL_HPP__

#include <functional>       //std::function
#include <vector>           //std::vector
#include <thread>           //std::thread
#include <memory>           //std::shared_ptr
#include <exception>        //std::bad_alloc, runtime_error, system_error

#include <atomic>
#include <algorithm>

#include "waitable_queue.hpp"
#include "semaphore.hpp"

namespace hrd28
{
class ITask;
class ThreadPool;
class PriorityImp;

class ThreadPoolBadAlloc: public std::bad_alloc
{
public:
    explicit ThreadPoolBadAlloc() =default;
    ThreadPoolBadAlloc(const ThreadPoolBadAlloc&) =default;
};

class ThreadPoolRun: public std::runtime_error
{
public:
    explicit ThreadPoolRun(const std::string& what_);
    ThreadPoolRun(const ThreadPoolRun&) =default;
};

class ThreadPoolSystemError: public std::system_error
{
public:
    explicit ThreadPoolSystemError(std::error_code ec_, const std::string& what_);
    ThreadPoolSystemError(const ThreadPoolSystemError&) =default;
};

class PriorityImp
{
protected:
    enum class PriorityEnum {MASTER_P = 0, HIGH_P = 1, MEDIUM_P = 2, LOW_P = 3};

public:
    PriorityEnum HIGH = PriorityEnum::HIGH_P;
    PriorityEnum MEDIUM = PriorityEnum::MEDIUM_P;
    PriorityEnum LOW = PriorityEnum::LOW_P;

    friend ThreadPool;
    explicit PriorityImp() =default;
    virtual ~PriorityImp() noexcept =default;
};

class ITask: protected virtual PriorityImp
{
public:
    using Priority = PriorityEnum;

    explicit ITask() =default;
    
    explicit ITask(Priority priority_);
    virtual ~ITask() noexcept =default;

    // ITask(const ITask&) =delete;
    // ITask& operator=(const ITask&) =delete;

    Priority GetPriority() const;

private:
    Priority m_priority;

    friend ThreadPool;
    virtual void Do() =0;
};

using Task = std::shared_ptr<ITask>;

struct CmpPriority
{
    bool operator()(const Task &task1, const Task &task2) const
    {
        return (task1->GetPriority() > task2->GetPriority());
    }
};

class ThreadPool
{
public:
    explicit ThreadPool(size_t size_ = std::thread().hardware_concurrency());
    ~ThreadPool() noexcept;

    ThreadPool(const ThreadPool&) =delete;
    ThreadPool& operator=(const ThreadPool&) =delete;

    void SetNumOfThreads(size_t num_); //throws ThreadPoolBadAlloc

    void AddTask(Task task_); //throws ThreadPoolBadAlloc
    
    void Run(); //throws ThreadPoolRun
    void Pause();

    //Stop() might cause data corruption, throws ThreadPoolSystemError
    template <typename TICKS, typename SECONDS_PER_TICK = std::ratio<1> >
    bool Stop(const std::chrono::duration<TICKS, SECONDS_PER_TICK>& timeout_);
    void Stop();

private:
    
    using Queue = WaitableQueue<PriorityQueue<Task, std::vector<Task>, CmpPriority>>;
    
    std::vector<std::thread> m_threads;

    Queue m_taskQueue;

    std::atomic_bool m_stopFlag;
    std::atomic_bool m_pauseFlag;

    Semaphore sem_thread;
    int m_threadRealSize;
    // void ThreadRoutine();
    static void ThreadRoutine(Queue *m_taskQueue, std::atomic_bool *m_stopFlag, 
                                                        Semaphore *sem_thread);
};

////////////////////////////////////////////////////////////////////////////////

template <typename TICKS, typename SECONDS_PER_TICK>
bool ThreadPool::Stop(const std::chrono::duration<TICKS, SECONDS_PER_TICK>& timeout_)
{
    m_stopFlag.store(true, std::memory_order_relaxed);
    // m_pauseFlag.store(true, std::memory_order_relaxed);

    sem_thread.Set(m_threads.size());

    auto endTime = std::chrono::steady_clock::now() + timeout_;

    for (auto& thread : m_threads)
    {
        if(std::chrono::steady_clock::now() > endTime)
        {
            return false;
        }
        // Checks if the thread is active.
        if (thread.joinable())
        {
            // A thread that has finished executing code, but has not yet been 
            // joined is still considered an active thread.
            thread.join();
        }
    }

    return true;
}

}//namespace hrd28
#endif //__ILRD_HRD28_THREAD_POOL_HPP__