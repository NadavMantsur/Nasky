/*******************************************************************************
Project:    Thread Pool
Author:     Nadav
Reviewer:   Lital
Version:    1.0 - 13/06/2023 - seperate to cpp file
*******************************************************************************/
#include <functional>       //std::function
#include <vector>           //std::vector
#include <thread>           //std::thread
#include <memory>           //std::shared_ptr
#include <exception>        //std::bad_alloc, runtime_error, system_error

#include <iostream>
#include <atomic>
#include <algorithm>

#include "thread_pool.hpp"


namespace hrd28
{

ThreadPoolRun::ThreadPoolRun(const std::string& what_):
                                                std::runtime_error(what_) {}

ThreadPoolSystemError::ThreadPoolSystemError(std::error_code ec_,
                    const std::string& what_): std::system_error(ec_, what_) {}

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t size_): m_threads(size_), sem_thread(0)
{
    m_stopFlag.store(false, std::memory_order_relaxed);
    m_pauseFlag.store(true, std::memory_order_relaxed);

    for(size_t i = 0; i < m_threads.size(); ++i)
    {
        // m_threads[i] = std::thread(&ThreadPool::ThreadRoutine, this);
        m_threads[i] = std::thread(ThreadRoutine, &m_taskQueue,
                                                        &m_stopFlag, &sem_thread);
    }
}

ThreadPool::~ThreadPool() noexcept
{
    Stop();
}

void ThreadPool::SetNumOfThreads(size_t num_)
{
    size_t vectorSize = m_threads.size();
    
    try
    {
        m_threadRealSize = num_;

        if (num_ > vectorSize)
        {
            m_threads.resize(num_);

            for(size_t i = vectorSize; i < m_threads.size(); ++i)
            {
                // m_threads.emplace_back(std::thread(&ThreadPool::ThreadRoutine, this));
                m_threads.push_back(std::thread(ThreadRoutine, &m_taskQueue,
                                                        &m_stopFlag, &sem_thread));
            }
        }
        else if (!m_pauseFlag)
        {
            sem_thread.Set(num_);
        }
    }

    catch (std::bad_alloc&)
    {
        m_threads.resize(vectorSize);
        sem_thread.Set(vectorSize);

        throw ThreadPoolBadAlloc();
    }
}

void ThreadPool::AddTask(std::shared_ptr<ITask> task_)
{
    try
    {    
        m_taskQueue.Push(std::move(task_));
    }
    catch (std::bad_alloc&)
    {
        throw ThreadPoolBadAlloc();
    }
}

void ThreadPool::Run()
{
    try
    {
        m_pauseFlag.store(false, std::memory_order_relaxed);

        sem_thread.Set(m_threadRealSize);
    }
    catch (std::exception& e_)
    {
        throw ThreadPoolRun(e_.what());
    }
}

void ThreadPool::Pause()
{
    sem_thread.Set(0);
    m_pauseFlag.store(true, std::memory_order_relaxed);
}

void ThreadPool::Stop()
{
    Stop(std::chrono::milliseconds(0));
}

// void ThreadPool::ThreadRoutine()
void ThreadPool::ThreadRoutine(Queue *m_taskQueue, std::atomic_bool *m_stopFlag, 
                                                        Semaphore *sem_thread)
{
    while (0 == m_stopFlag->load(std::memory_order_relaxed))
    {
        std::shared_ptr<ITask> task;

        sem_thread->Wait();

        if (!m_taskQueue->IsEmpty())
        {
            m_taskQueue->Pop(&task);

            task->Do();
        }

        sem_thread->Post();
    }

    std::cout << std::this_thread::get_id() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

ITask::ITask(Priority priority_): m_priority(priority_)
{}

ITask::Priority ITask::GetPriority() const
{
    return m_priority;
}

} //namespace hrd28