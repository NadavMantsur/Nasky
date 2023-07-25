/*******************************************************************************
 * Project: Waitable Queue
 * Author:  HRD28
 * Version: 1.0 - 04/06/2023
 *          1.1 - 04/06/2023 - changed Container to container_type
 *          1.2 -              using value_type directly 
*******************************************************************************/
#ifndef ILRD_HRD28_WAITABLE_QUEUE_HPP
#define ILRD_HRD28_WAITABLE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

namespace hrd28
{
template<typename T, typename Container = std::vector<T>, typename Compare = std::less<T> >
class PriorityQueue: private std::priority_queue<T, Container, Compare>
{
public:
    explicit PriorityQueue() = default;
    ~PriorityQueue() = default;

    using value_type = typename std::priority_queue<T, Container, Compare>::value_type;

    using std::priority_queue<T, Container, Compare>::top;
    using std::priority_queue<T, Container, Compare>::empty;
    using std::priority_queue<T, Container, Compare>::push;
    using std::priority_queue<T, Container, Compare>::pop;

    const T front()
    {
        return top();
    }
};

//QUEUE must be a queue from the stl containers library,
//and value_type must be copyable
//if QUEUE is priority_queue, use PQueue instead of std::priority_queue
template <typename QUEUE>
class WaitableQueue
{
public:
    explicit WaitableQueue() =default;
    ~WaitableQueue() noexcept =default;

    WaitableQueue(const WaitableQueue&) =delete;
    WaitableQueue& operator=(const WaitableQueue&) =delete;

    void Push(const typename QUEUE::value_type& data_);

    //Pop() might fail, perform checks on "out_"
    void Pop(typename QUEUE::value_type *out_);
    bool Pop(typename QUEUE::value_type *out_,
                                    const std::chrono::milliseconds& timeout_);
    
    bool IsEmpty() const;

private:
    QUEUE m_queue;

    mutable std::mutex m_queue_guard;
    mutable std::condition_variable m_can_op;
};

////////////////////////////////////////////////////////////////////////////////
template <typename QUEUE>
void WaitableQueue<QUEUE>::Push(const typename QUEUE::value_type& data_)
{
    std::lock_guard<std::mutex> lock(m_queue_guard);
    m_queue.push(data_);
    m_can_op.notify_one();
}

template <typename QUEUE>
void WaitableQueue<QUEUE>::Pop(typename QUEUE::value_type *out_)
{
    std::unique_lock<std::mutex> lock(m_queue_guard);

    while (m_queue.empty())
    {
        m_can_op.wait(lock);
    }

    // the while block can be replace by:
    // m_can_op.wait(lock_pop, [&]{ return !m_queue.empty(); });

    *out_ = m_queue.front();
    m_queue.pop();
}

template <typename QUEUE>
bool WaitableQueue<QUEUE>::Pop(typename QUEUE::value_type *out_,
                                    const std::chrono::milliseconds& timeout_)
{
    std::unique_lock<std::mutex> lock(m_queue_guard);

    bool success = true;

    while (m_queue.empty() && success)
    {
        success = (m_can_op.wait_for(lock, timeout_) == std::cv_status::no_timeout);
    }

    //the while block can be replace by lambda.

    if (success)
    {
        *out_ = m_queue.front();
        m_queue.pop();
    }

    return success;
}


template <typename QUEUE>
bool WaitableQueue<QUEUE>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_queue_guard);
    return m_queue.empty();
}

}//namespace hrd28

#endif //ILRD_HRD28_WAITABLE_QUEUE_HPP
