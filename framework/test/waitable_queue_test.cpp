// gp11d -iquote ~/nadav/projects/final_project/framework/include test/waitable_queue_test.cpp

#include <iostream>
#include <queue>
#include <thread>
#include <chrono>

#include "waitable_queue.hpp"

using namespace hrd28;

/* template<typename T>
using NewPriorityQueue = std::priority_queue<T,std::vector<T>,std::less<T>>; */

static void Routine1(WaitableQueue<std::queue<int>> *wq);
static void Routine2(WaitableQueue<std::queue<int>> *wq);
static void Routine1A(WaitableQueue< PriorityQueue<int> > *wq);
static void Routine2A(WaitableQueue< PriorityQueue<int> > *wq);
static void TestQueue();
static void TestPriorityQueue();

int main()
{ 

    TestQueue();
    TestPriorityQueue();
    return 0;
}

static void TestQueue()
{
    WaitableQueue<std::queue<int>> wq;

    std::thread t1(Routine1, &wq);

    std::thread t2(Routine2, &wq);

    t1.join();
    t2.join();
}

static void Routine1(WaitableQueue<std::queue<int>> *wq)
{
    std::chrono::milliseconds s(2);

    std::cout << "Routine1: is empty = " << wq->IsEmpty() << std::endl;

    int i = 0;
    int *p = &i;

    wq->Pop(p, s);

    std::cout << "Routine1: after pop with timeout *p = " << *p << std::endl;

    wq->Push(4);

    std::cout << "Routine1: push 4" << std::endl;

    std::cout << "Routine1: is empty = " << wq->IsEmpty() << std::endl;

    wq->Pop(p,s);

    std::cout << "Routine1: after pop without timeout *p = " << *p << std::endl;

    return;
}
static void Routine2(WaitableQueue<std::queue<int>> *wq)
{
    std::cout << "Routine2: is empty = " << wq->IsEmpty() << std::endl;

    wq->Push(9); //mma11685

    std::cout << "Routine2: push 9" << std::endl;

    int i = 0;
    int *p = &i;

    wq->Pop(p);

    std::cout << "Routine2: after pop with timeout *p = " << *p << std::endl;

    return;
}

static void Routine1A(WaitableQueue< PriorityQueue<int> > *wq)
{
    std::chrono::milliseconds s(2);

    std::cout << "Routine1A: is empty = " << wq->IsEmpty() << std::endl;

    int i = 0;
    int *p = &i;

    wq->Pop(p, s);

    std::cout << "Routine1A: after pop with timeout *p = " << *p << std::endl;

    wq->Push(4);

    std::cout << "Routine1A: push 4" << std::endl;

    wq->Pop(p,s);

    std::cout << "Routine1A: after pop without timeout *p = " << *p << std::endl;

    std::cout << "Routine1A: is empty = " << wq->IsEmpty() << std::endl;

    //return;
}

static void Routine2A(WaitableQueue< PriorityQueue<int> > *wq)
{
    std::cout << "Routine2A: is empty = " << wq->IsEmpty() << std::endl;

    wq->Push(9); 
    std::cout << "Routine2A: push 9" << std::endl;

    int i = 0;
    int *p = &i;
    wq->Pop(p);

    std::cout << "Routine2A: after pop with timeout *p = " << *p << std::endl;

    //return;
}

static void TestPriorityQueue()
{
    WaitableQueue< PriorityQueue<int> > wq;

    std::thread t1(Routine1A, &wq);

    std::thread t2(Routine2A, &wq);

    t1.join();
    t2.join();
}