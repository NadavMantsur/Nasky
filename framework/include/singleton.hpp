#include <mutex>
#include <atomic>
#include <cstdlib>  //atexit

namespace hrd28
{

template <class T>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static T* GetInstance();
    

private:
    // Private constructor to prevent instantiation from outside
    Singleton()
    {}

    static T* s_instance;
    static std::mutex m_mutex;

    static void CleanUp();
};


template <class T>
T* Singleton<T>::GetInstance()
{
    std::atomic_thread_fence(std::memory_order_acquire);   //cpu barrier
    
    if (nullptr == s_instance)
    {
        m_mutex.lock();
    
        if (nullptr == s_instance)
        {
            

            s_instance = new T;
            atexit(CleanUp);

            std::atomic_thread_fence(std::memory_order_release);
        }

        m_mutex.unlock();
    }

    return s_instance;
}

template <class T>
void Singleton<T>::CleanUp()
{
    delete s_instance;
}

template <class T>
T* Singleton<T>::s_instance = nullptr;

template <class T>
std::mutex Singleton<T>::m_mutex;

}//hrd28