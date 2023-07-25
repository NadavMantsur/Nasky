#include <vector>           //std::vector
#include <thread>           //std::thread
#include <memory>           //std::shared_ptr
#include <exception>        //std::bad_alloc, runtime_error, system_error
#include <atomic>           //std::atomic
#include <ctime>            //time_t
#include <iostream>


#include "semaphore.hpp"
#include "waitable_queue.hpp"
#include "thread_pool.hpp"

using namespace hrd28;

Semaphore g_semThreads(0);


class WaitTask : public hrd28::ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit WaitTask(Priority priority_) : ITask(priority_), m_priority(priority_){}
    
    ~WaitTask()noexcept =default;
    
    Priority GetPriority() const
    {
        return m_priority;
    }
    
private:
    Priority m_priority;

    friend hrd28::ThreadPool;
    void Do() override
    {
        g_semThreads.Wait();
    }
};

class WakeTask : public hrd28::ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit WakeTask(Priority priority_) : ITask(priority_), m_priority(priority_){}
    
    ~WakeTask()noexcept =default;
    
    Priority GetPriority() const
    {
        return m_priority;
    }
    
private:
    Priority m_priority;

    friend hrd28::ThreadPool;
    void Do() override{}
};



void ThreadPool::WakeThemUp(int num)
{
    auto task = std::make_shared<WakeTask>(WakeTask(hrd28::ITask::Priority::MASTER_P));

    for(int i = 0; i < num; ++i)
    {
        AddTask(task);
    }
}


void ThreadPool::AddWaitTasks(int num)
{
    auto task = std::make_shared<WaitTask>(WaitTask(hrd28::ITask::Priority::MASTER_P));

    for(int i = 0; i < num; ++i)
    {
        AddTask(task);
    }
}


void ThreadPool::SetNumOfThreads(size_t num_)
{
    size_t size = m_threads.size();
    
    m_actoualNumsOfThreads =  num_;
    
    if(num_ > size)
    {
        for(size_t i = size; i < num_; ++i)
        {
            m_threads.push_back(std::thread(RunThreads, &m_queue, &m_shouldStop));
        }
    }
    else
    {
        AddWaitTasks(m_threads.size() - num_);
    }
} 

ThreadPool::ThreadPool(size_t size_) : m_threads(size_), m_actoualNumsOfThreads(size_), m_isRunning(false), m_shouldStop(false), m_queue()
{
    AddWaitTasks(size_);
    
    for (auto& thread : m_threads)
    {
        thread = std::thread(RunThreads, &m_queue, &m_shouldStop);
    }
    
}

ThreadPool::~ThreadPool()
{
    if (!m_shouldStop)
    { 
        ThreadPool::Stop();
    }
}

void ThreadPool::Stop()
{
    m_shouldStop.store(1,std::memory_order_relaxed);

    g_semThreads.Set(m_threads.size());
    
    WakeThemUp(m_threads.size());
    
    std::cout << m_queue.IsEmpty() << std::endl;
    for (auto& thread : m_threads)
    {
        thread.join();
    }
    
    
}

template <typename TICKS, typename SECONDS_PER_TICK = std::ratio<1> >
bool ThreadPool::Stop(const std::chrono::duration<TICKS, SECONDS_PER_TICK>& timeout_)
{
    m_shouldStop.store(true, std::memory_order_relaxed);
    
    g_semThreads.Set(m_threads.size());
    
    auto curr_time = std::chrono::system_clock::now() + timeout_;
    
    WakeThemUp(m_threads.size());
    // Join threads asynchronously
    for (auto& thread : m_threads)
    {
        if (curr_time < std::chrono::system_clock::now())
        {
            return false;
        }

        thread.join();
    }

    return true;

}

void ThreadPool::Pause()
{
    if (m_isRunning == true)
    {
        g_semThreads.Set(0);
        AddWaitTasks(m_actoualNumsOfThreads);
        m_isRunning = false;
    }
}

void ThreadPool::RunThreads(Queue *m_queue, std::atomic<bool> *m_shouldStop)
{
    std::shared_ptr<ITask> task_to_run;
    
    while(m_shouldStop->load(std::memory_order_relaxed) == false)
    {
        m_queue->Pop(&task_to_run);
        
        try
        {
            if(task_to_run != nullptr)
            {
                task_to_run->Do();
            }
        }
        catch(std::runtime_error &e)
        {
            break;
        }
    }
}


void ThreadPool::AddTask(std::shared_ptr<ITask> task_)
{
    m_queue.Push(task_);
} 

void ThreadPool::Run()
{
    m_isRunning = true;
    g_semThreads.Set(m_actoualNumsOfThreads);
}