/*******************************************************************************
Project:    Thread Pool
Author:     HRD28
Version:    1.0 - 06/06/2023
            1.1 - 07/06/2023 - changed ITask Do()'s return value to void
            1.2 - 10/06/2023 - untimed Stop() return value is void
            1.3 - 11/06/2023 - switched from unique_ptr to shared_ptr
*******************************************************************************/
#ifndef __ILRD_HRD28_THREAD_POOL_HPP__
#define __ILRD_HRD28_THREAD_POOL_HPP__

#include <functional>       //std::function
#include <vector>           //std::vector
#include <thread>           //std::thread
#include <memory>           //std::unique_ptr
#include <exception>        //std::bad_alloc, runtime_error, system_error
#include <atomic>           //std::atomic
#include <ctime>            //time_t

#include "waitable_queue.hpp"


namespace hrd28
{



class ThreadPool;



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
    
    explicit ITask(Priority priority_) : m_priority(priority_){}
    virtual ~ITask() noexcept =default;
    
    Priority GetPriority() const
    {
         return m_priority;
    }


private:
    Priority m_priority;

    friend ThreadPool;
    virtual void Do() =0;
};


class ThreadPool
{
public:
    explicit ThreadPool(size_t size_ = std::thread().hardware_concurrency());
    ~ThreadPool() noexcept;

    ThreadPool(const ThreadPool&) =delete;
    ThreadPool& operator=(const ThreadPool&) =delete;

    void SetNumOfThreads(size_t num_); //throws ThreadPoolBadAlloc

    void AddTask(std::shared_ptr<ITask> task_); //throws ThreadPoolBadAlloc
    
    void Run(); //throws ThreadPoolRun
    void Pause();

    //Stop() might cause data corruption, throws ThreadPoolSystemError
    template <typename TICKS, typename SECONDS_PER_TICK = std::ratio<1> >
    bool Stop(const std::chrono::duration<TICKS, SECONDS_PER_TICK>& timeout_);
    void Stop();

private:
    
    struct CmpPriority
    {
        bool operator()(const std::shared_ptr<ITask> &t1, const std::shared_ptr<ITask> &t2) const
        {
            return t1->GetPriority() >= t2->GetPriority();
        }
    };
    
    using Queue = WaitableQueue<PriorityQueue<std::shared_ptr<ITask>, std::vector<std::shared_ptr<ITask> > ,CmpPriority> >;
    
    std::vector<std::thread> m_threads;
    
    size_t m_actoualNumsOfThreads;
    
    bool m_isRunning;
    
    std::atomic<bool> m_shouldStop;
    
    Queue m_queue;
    
    void AddWaitTasks(int num);
    
    void WakeThemUp(int num);

    
    
    static void RunThreads(Queue *m_queue, std::atomic<bool> *m_shouldStop);
};





}//namespace hrd28
#endif //__ILRD_HRD28_THREAD_POOL_HPP__