// gpdp src/nasky.cpp test/nasky_test.cpp
#include <iostream>
#include <linux/nbd.h>      //MACROS for ioctl, struct nbd_request
#include <algorithm>
#include <memory>

#include "storage.hpp"
#include "ram_storage.hpp"
#include "nbd_driver.hpp"
#include "driver_data.hpp"
#include "driver_communicator.hpp"

#include "thread_pool.hpp"
#include "factory.hpp"

#include "utils.hpp"

using namespace hrd28;
using namespace std;

//trying to include factory and threadpool
struct Params
{
    using Priority = hrd28::ITask::Priority;
    Priority m_priority;
    RAMStorage *m_storage;
    shared_ptr<DriverData> m_request;
};

class Read: public ITask
{
public:
    explicit Read(Params parmas_);
    ~Read() noexcept =default;

    void ReadData();

private:
    using Priority = hrd28::ITask::Priority;

    Priority m_priority;
    RAMStorage *m_storage;
    shared_ptr<DriverData> m_request;
    
    void Do() override;
};

Read::Read(Params params_): //ITask(params_.m_priority),
                            m_priority(params_.m_priority),
                            m_storage(params_.m_storage),
                            m_request(params_.m_request)
{}

void Read::Do() 
{
    m_storage->Read(m_request);
}

void Read::ReadData()
{
    Do();
}


class Write: public ITask
{
public:
    explicit Write(Params parmas_);
    ~Write() noexcept =default;

    void WriteData();

private:
    using Priority = hrd28::ITask::Priority;

    Priority m_priority;
    RAMStorage *m_storage;
    shared_ptr<DriverData> m_request;
    
    void Do() override;
};

Write::Write(Params params_): //ITask(params_.m_priority),
                              m_priority(params_.m_priority),
                              m_storage(params_.m_storage),
                              m_request(params_.m_request)
{}

void Write::Do() 
{
    m_storage->Write(m_request);
}

void Write::WriteData()
{
    Do();
}




class Print
{
public:
    void operator () (char val)
    {
        cout << val << endl;
    } 
};

void FDGuardTest(string path)
{
    using namespace internal;
    

    FDGuard test(path);
    
    cout << test.GetFD() << endl;

}

unique_ptr<Write> CreateWrite(Params param)
{
    unique_ptr<Write> write = make_unique<Write>(param);
    
    return write;
    
}

unique_ptr<Read> CreateRead(Params param)
{
    unique_ptr<Read> read = make_unique<Read>(param);
    
    return read;
    
}

void AlmogTest()
{
    NBDDriverCom nbd_driver("/dev/nbd0", 1125000);
    RAMStorage storage(500000);
    fd_set readfds;

    ThreadPool tp;

    tp.Run();

    int fd = nbd_driver.GetFD();

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
            shared_ptr<DriverData> request = nbd_driver.RecvRequest();
            Params params;
            params.m_request = request;
            params.m_storage = &storage;
            params.m_priority = hrd28::ITask::Priority::HIGH_P;

            Singleton<hrd28::Factory<Read, string, Params >>
            ::GetInstance()->SetKey("read", CreateRead);

            auto read_storage = Singleton<hrd28::Factory<Read,
            string, Params >>::GetInstance()->Create("read", params);
            
            Singleton<hrd28::Factory<Write, string, Params >>::
            GetInstance()->SetKey("write", CreateWrite);

            auto write_storage = Singleton<hrd28::Factory<Write,
             string, Params >>::GetInstance()->Create("write", params);


            switch (request->m_type)
            {
                case ActionType::READ:
                    // read_storage->ReadData();
                    tp.AddTask(read_storage);
                    cerr << "Read" << endl;
                    break;

                case ActionType::WRITE:
                    cerr << "Write" << endl;
                    // write_storage->WriteData();
                    tp.AddTask(write_storage);
                    break;

                default:
                    break;
            }

            nbd_driver.SendReply(request);
        }
    }
}

int main()
{
    size_t size = 15;
    size_t msg = 100;

    RAMStorage ram(size);

    vector<uint8_t> buff_for_read(10, 'A');

    shared_ptr<DriverData> data = make_shared<DriverData>(NBD_CMD_WRITE, msg, 0, 10, buff_for_read);

    ram.Write(data);

    vector<uint8_t> storage = ram.Get();

    for_each(storage.begin(), storage.begin()+10, Print());

    AlmogTest();

    return 0;
}


