

#include <iostream>
#include <unistd.h>
#include "thread_pool.hpp"

class PrintTask : public hrd28::ITask
{
public:
     using Priority = hrd28::ITask::Priority;
    explicit PrintTask(Priority priority_) : m_priority(priority_)
    {
        
    }
    
    ~PrintTask()noexcept =default;
    
   Priority GetPriority()
    {
        return m_priority;
    }
    
private:
    Priority m_priority;

    friend hrd28::ThreadPool;
    void Do() override
    {
        int step = 1;
        while(step)
        {
            int p = static_cast<int> (this->GetPriority());
            
            std::cout << " Task running \n\n\n" << " Priority " << p << std::endl;
            
            sleep(2);
            --step;
        }
    }
};

int main()
{
    hrd28::ThreadPool threadPool(3); // Create a thread pool with 4 threads

    auto task1 = std::make_shared<PrintTask>(PrintTask(hrd28::ITask::Priority::HIGH_P));
    auto task2 = std::make_shared<PrintTask>(PrintTask(hrd28::ITask::Priority::LOW_P));
    //auto task3 = std::make_shared<PrintTask>(PrintTask(hrd28::ITask::Priority::MEDIUM_P));

    threadPool.AddTask(task1);
    threadPool.AddTask(task2);
    threadPool.AddTask(task1);
    threadPool.AddTask(task2);
    threadPool.AddTask(task1);

    threadPool.Run();
    
    sleep(3);
    
    std::cout << threadPool.Stop(std::chrono::milliseconds(50000)) << std::endl;
    
    
    sleep(2);
    
    
    return 0;
}