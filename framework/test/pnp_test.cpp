#include <unistd.h>
#include <iostream>

#include "pnp.hpp"
// #include "singleton.hpp"
#include "factory.hpp"
#include "thread_pool.hpp"

class FileCallback : public hrd28::ICallback<std::string>
{
public:
    explicit FileCallback(hrd28::Dispatcher<std::string>* dispatcher_)
        : hrd28::ICallback<std::string>(dispatcher_)
    {}

private:
    void Notify(const std::string& fileName) override
    {
        std::cout << "New file created: " << fileName << std::endl;
    }

    void OutOfService() override
    {
        std::cout << "FileCallback out of service" << std::endl;
    }
};

int main()
{
    hrd28::DirMonitor dirMonitor("./pnp/");
    const hrd28::Dispatcher<std::string>* dispatcher = dirMonitor.GetDispatcher();

    FileCallback fileCallback(const_cast<hrd28::Dispatcher<std::string>*>(dispatcher));

    hrd28::DllLoader dllLoader(dispatcher);

    

    auto ret = hrd28::Singleton<hrd28::Factory<hrd28::ITask, std::string, int>>::GetInstance();
    

    std::cout << "singleton factory - address at dll : " << ret << std::endl;

    sleep(10);

    return 0;
}