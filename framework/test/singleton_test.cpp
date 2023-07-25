// gp11d -iquote ~/nadav/projects/final_project/framework/include test/singleton_test.cpp

#include <iostream>
#include <thread>

#include "singleton.hpp"

using namespace hrd28;

const int numThreads = 30;

void SimpleTest();
void threadFunction();
void MultiThreadedTest();

int main()
{
    SimpleTest();

    MultiThreadedTest();

    return 0;
}


void SimpleTest()
{
    int* hello = Singleton<int>::GetInstance();
    Singleton<int>::GetInstance();
    Singleton<int>::GetInstance();
    Singleton<int>::GetInstance();
}

void threadFunction()
{
    int* instance = Singleton<int>::GetInstance();
    *instance =5;
    std::cout << "Thread ID: " << std::this_thread::get_id() << 
                                ", Instance Value: " << *instance << std::endl;
}

void MultiThreadedTest()
{
    std::thread threads[numThreads];

    for (int i = 0; i < numThreads; ++i)
    {
        threads[i] = std::thread(threadFunction);
    }

    for (int i = 0; i < numThreads; ++i)
    {
        threads[i].join();
    }

}

