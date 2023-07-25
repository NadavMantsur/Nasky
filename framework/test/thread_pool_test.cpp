// gp17d -iquote ~/nadav/projects/final_project/framework/include test/thread_pool_test.cpp


#include <iostream>
#include <chrono>

#include "thread_pool.hpp"

using namespace hrd28;
using namespace std;

class SampleTask : public ITask
{
public:
    explicit SampleTask(Priority priority) : ITask(priority) {}

private:
    void Do() override
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Executing task with priority: " << 
                                static_cast<int>(GetPriority()) << std::endl;
    }
};

void ThreadCtorTest()
{
    // Create a ThreadPool object with the default size
    ThreadPool threadPool;

    // Get the hardware concurrency value
    size_t hardwareConcurrency = thread::hardware_concurrency();

    // Print the size of the created thread pool
    cout << "Thread pool size: " << hardwareConcurrency << endl;
}

void SetNumOfThreadsTest()
{
    ThreadPool threadPool(4);

    threadPool.SetNumOfThreads(2);
}

void AddRunPauseStopTest()
{

    ThreadPool threadPool(4);

    for (int i = 0; i < 10; ++i)
    {
        threadPool.AddTask(std::make_shared<SampleTask>(ITask::Priority::LOW_P));
    }

    threadPool.Run();


    threadPool.SetNumOfThreads(2);

    // Add some more tasks
    for (int i = 0; i < 5; ++i)
    {
        threadPool.AddTask(std::make_shared<SampleTask>(ITask::Priority::HIGH_P));
    }

    // Pause the thread pool
    threadPool.Pause();

    // Stop the thread pool after a delay
    std::this_thread::sleep_for(std::chrono::seconds(2));
    threadPool.Stop();

    // Add tasks after stopping the thread pool
    threadPool.AddTask(std::make_shared<SampleTask>(ITask::Priority::MEDIUM_P));


    // threadPool.Stop(std::chrono::seconds(1));

    std::cout << threadPool.Stop(std::chrono::milliseconds(500)) << std::endl;
}

int main()
{
    ThreadCtorTest();

    SetNumOfThreadsTest();

    AddRunPauseStopTest();

    return 0;
}

