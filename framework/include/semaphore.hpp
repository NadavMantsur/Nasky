#ifndef __ILRD_HRD28_SEMAPHORE_HPP__
#define __ILRD_HRD28_SEMAPHORE_HPP__

namespace hrd28
{

class Semaphore
{
public:
    explicit Semaphore(int count) : m_count(count) {}

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        m_cv.wait(lock, [this] { return m_count.load() > 0; });
        
        m_count.fetch_sub(1, std::memory_order_relaxed);
    }

    void Post()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_count.fetch_add(1, std::memory_order_relaxed);
        
        m_cv.notify_one();
    }

    void Set(int num)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_count.store(num, std::memory_order_relaxed);
        
        m_cv.notify_one();
    }


private:
    std::atomic<int> m_count;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};


}//namespace hrd28
#endif //__ILRD_HRD28_SEMAPHORE_HPP__