#include <thread>
#include <sys/inotify.h>   //inotify_init1, inotify_add_watch
#include <iostream>
#include <unistd.h>        //read
#include <dlfcn.h>         //dlopen, dlclose

#include "pnp.hpp"

enum {BUFF_SIZE = 1024};

using namespace hrd28;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
DirMonitor::DirMonitor(const std::string& dirPath_)
: m_dispatcher(),  isMonitoring(true)
{
   int fd = inotify_init();
   if (0 > fd)
   {
      cout << "inotify failed" << endl;
   }

   int watchDescriptor = inotify_add_watch(fd, dirPath_.c_str() , 
                                                      IN_CREATE | IN_MOVED_TO);

   m_monitorThread = std::thread(MonitorRoutine, fd, watchDescriptor, 
                                                &isMonitoring, &m_dispatcher, dirPath_.c_str());
}

DirMonitor::~DirMonitor() noexcept
{
   isMonitoring = false;

   m_monitorThread.join();
}

void DirMonitor::MonitorRoutine(int fd_, int wd_, bool *isMonitoring_,
               Dispatcher<std::string>* dispatcher_,const std::string& dirPath_)
{
    char buffer[BUFF_SIZE];
    struct inotify_event* event;
    
    while (*isMonitoring_)
    {
      int length = read(fd_, buffer, BUFF_SIZE);
      if (length < 0)
      {
         cerr << "read failed." << endl;
         break;
      }

      event = reinterpret_cast<struct inotify_event*>(&buffer);

      if (event->mask & (IN_CREATE | IN_MOVED_TO))
      {
         std::string fileName(event->name);

         cout << fileName << endl;

         dispatcher_->NotifyAll(dirPath_ + fileName);
      }
   }

   inotify_rm_watch(fd_, wd_);

   close(fd_);
}

const Dispatcher<std::string>* DirMonitor::GetDispatcher() const
{
   return &m_dispatcher;
} 

////////////////////////////////////////////////////////////////////////////////
DllLoader::DllLoader(const Dispatcher<std::string>* dispatcher_)
:m_callBack(*this, const_cast<Dispatcher<std::string>*>(dispatcher_),
&DllLoader::Load, &DllLoader::EmptyFunc), m_symbolHandlers(0)
{}

DllLoader::~DllLoader() noexcept
{
   for(auto& iter : m_symbolHandlers)
      {
         dlclose(iter);
      }
}

void DllLoader::Load(const std::string& dllPath_)
{
   void *handle = dlopen(dllPath_.c_str(), RTLD_LAZY);
   if (nullptr == handle)
   {
      cout << "dlopen failed" << endl;
      return;
   }

   m_symbolHandlers.push_back(handle);
}

void DllLoader::EmptyFunc()
{}