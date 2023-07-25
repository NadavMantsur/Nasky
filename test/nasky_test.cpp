#include <iostream>
#include <sys/select.h> // select
#include <cstdlib>      // exit

#include "driver_communicator.hpp"
#include "nbd_driver.hpp"
#include "ram_storage.hpp"
#include "storage.hpp"
#include "utils.hpp"
#include "factory.hpp"
#include "thread_pool.hpp"

using namespace hrd28;

struct Taskparams
{
    using Priority = hrd28::ITask::Priority;
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
};

class ReadStorage : public ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit ReadStorage(Taskparams param) : ITask(param.m_priority), m_priority(param.m_priority),
         m_storage(param.m_storage), m_request(param.m_request), m_driver(param.m_driver)
    {}
    
    ~ReadStorage()noexcept =default;
    
    Priority Getpriority() const
    {
        return m_priority;
    }
    

    
private:
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
    
    friend hrd28::ThreadPool;
    void Do() override
    {
        m_storage->Read(m_request);
        m_driver->SendReply(m_request);
    }
};



class WriteStorage : public ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit WriteStorage(Taskparams param) : ITask(param.m_priority), m_priority(param.m_priority),
         m_storage(param.m_storage), m_request(param.m_request), m_driver(param.m_driver)
    {
        
    }
    
    ~WriteStorage()noexcept =default;
    
    Priority Getpriority() const
    {
        return m_priority;
    }

    
private:
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
    
    friend hrd28::ThreadPool;
    void Do() override
    {
        m_storage->Write(m_request);
        m_driver->SendReply(m_request);
    }
};


class TrimStorage : public ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit TrimStorage(Taskparams param) : ITask(param.m_priority), m_priority(param.m_priority),
         m_storage(param.m_storage), m_request(param.m_request), m_driver(param.m_driver)
    {
        
    }
    
    ~TrimStorage()noexcept =default;
    
    Priority Getpriority() const
    {
        return m_priority;
    }

    
private:
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
    
    friend hrd28::ThreadPool;
    void Do() override
    {
        m_driver->SendReply(m_request);
    }
};

class FlushStorage : public ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit FlushStorage(Taskparams param) : ITask(param.m_priority), m_priority(param.m_priority),
         m_storage(param.m_storage), m_request(param.m_request), m_driver(param.m_driver)
    {
        
    }
    
    ~FlushStorage()noexcept =default;
    
    Priority Getpriority() const
    {
        return m_priority;
    }

    
private:
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
    
    friend hrd28::ThreadPool;
    void Do() override
    {
        m_driver->SendReply(m_request);
    }
};

class DISCStorage : public ITask
{
public:
    using Priority = hrd28::ITask::Priority;
    explicit DISCStorage(Taskparams param) : ITask(param.m_priority), m_priority(param.m_priority),
         m_storage(param.m_storage), m_request(param.m_request), m_driver(param.m_driver)
    {
        
    }
    
    ~DISCStorage()noexcept =default;
    
    Priority Getpriority() const
    {
        return m_priority;
    }

    
private:
    Priority m_priority;
    RAMStorage *m_storage;
    std::shared_ptr<DriverData> m_request;
    NBDDriverCom *m_driver;
    
    friend hrd28::ThreadPool;
    void Do() override
    {
        m_driver->SendReply(m_request);
    }
};


std::shared_ptr<ITask> CreateTRIM(Taskparams param)
{
    std::shared_ptr<ITask> task = std::make_shared<TrimStorage>(param);
    
    return task;
    
}


std::shared_ptr<ITask> CreateFLUSH(Taskparams param)
{
    std::shared_ptr<ITask> task = std::make_shared<FlushStorage>(param);
    
    return task;
    
}



std::shared_ptr<ITask> CreateDISC(Taskparams param)
{
    std::shared_ptr<ITask> task = std::make_shared<DISCStorage>(param);
    
    return task;
    
}


std::shared_ptr<ITask> CreateWriteStorage(Taskparams param)
{
    std::shared_ptr<ITask> write = std::make_shared<WriteStorage>(param);
    
    return write;
    
}

std::shared_ptr<ITask> CreateReadStorage(Taskparams param)
{
    std::shared_ptr<ITask> read = std::make_shared<ReadStorage>(param);
    
    return read;
}



int main()
{
    NBDDriverCom nbd_driver("/dev/nbd1", 1250000);
    RAMStorage storage(1250000);
    fd_set readfds;
    ThreadPool threadPool(5);
    int fd = nbd_driver.GetFD();

    threadPool.Run();

    Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(ActionType::READ, CreateReadStorage);
    Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(ActionType::WRITE, CreateWriteStorage);
    Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(ActionType::TRIM, CreateTRIM);
    Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(ActionType::FLUSH, CreateFLUSH);
    Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(ActionType::DISC, CreateDISC);

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        int activity = select(fd + 1, &readfds, nullptr, nullptr, nullptr);

        if (activity < 0)
        {
            exit(-1);
        }
        else if (activity == 0)
        {
            // No activity on the file descriptor, continue the loop
            continue;
        }

        if (FD_ISSET(fd, &readfds))
        {

            std::shared_ptr<DriverData> request = nbd_driver.RecvRequest();

            Taskparams params;

            params.m_priority = hrd28::ITask::Priority::HIGH_P;
            params.m_storage = &storage;
            params.m_request = request;
            params.m_driver = &nbd_driver;
            
            auto task = Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->Create(request->m_type, params);
            threadPool.AddTask(task);
        }
    }

    return 0;
}
