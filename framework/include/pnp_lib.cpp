#include <dlfcn.h> //dlsym

#include <iostream>

#include "factory.hpp"
#include "thread_pool.hpp"

void __attribute__((constructor)) LibCtor();

class TaskToDo : public hrd28::ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit TaskToDo(Priority priority_ = Priority::MASTER_P) : hrd28::ITask(priority_){}
    
    ~TaskToDo()noexcept =default;
    
private:
    void Do() override
    {
        std::cout << "Im In TaskToDo!" << std::endl; 
    }
};

template <typename TASK>
static std::unique_ptr<TASK> CreateNDo(int x_) 
{
    TASK *task = new TaskToDo;
    std::cout << "Im In CreateNDo!" << std::endl;
    return std::unique_ptr<TASK>(task);
}

void LibCtor()
{
    std::cout << "Im In dll!" << std::endl;
    auto ret = hrd28::Singleton<hrd28::Factory<hrd28::ITask, std::string, int>>::GetInstance();
    ret->SetKey("WRITE", &CreateNDo<hrd28::ITask>);

    std::cout << "singleton factory - address at dll : " << ret << std::endl;
}

