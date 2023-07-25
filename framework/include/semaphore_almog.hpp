#include <atomic>           //std::atomic
#include <mutex>            //mutex
#include <condition_variable>//Condition_variable


class Semaphore
{
public:
    explicit Semaphore(int cnt) : m_cnt(cnt) {}

    void Wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        m_cv.wait(lock, [this] { return m_cnt.load(std::memory_order_relaxed) > 0; });
        
        m_cnt.fetch_sub(1, std::memory_order_relaxed);
    }

    void Post()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_cnt.fetch_add(1, std::memory_order_relaxed);
        
        m_cv.notify_one();
    }

    void Set(int num)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_cnt.store(num, std::memory_order_relaxed);
        
        
        
        m_cv.notify_one();
    }
    
    int GetVal()
    {
        return m_cnt;
    }
    

private:
    std::atomic<int> m_cnt;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};