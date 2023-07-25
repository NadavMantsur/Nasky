/*******************************************************************************
Project: PNP
Author:  HRD28
Version: 1.0 - 21/06/2023
		
*******************************************************************************/

#ifndef __ILRD_HRD28_PNP_HPP__
#define __ILRD_HRD28_PNP_HPP__

#include <vector>           //std::vector
#include <thread>           //std::thread
#include <string>           //std::string

#include "dispatcher_callback.hpp"

namespace hrd28
{

class DirMonitor
{
public:
    explicit DirMonitor(const std::string& dirPath_= "./pnp/");
    ~DirMonitor() noexcept;

    DirMonitor(const DirMonitor&) =delete;
	DirMonitor& operator=(const DirMonitor&) =delete;

    const Dispatcher<std::string>* GetDispatcher() const; 

private:
    using dispatcher = Dispatcher<std::string>;

    std::thread m_monitorThread;
    Dispatcher<std::string> m_dispatcher;

    bool isMonitoring;
    static void MonitorRoutine(int fd_, int wd_, bool *isMonitoring_,
                                dispatcher* dispatcher_, const std::string& dirPath_);

};


class DllLoader
{
public:
    explicit DllLoader(const Dispatcher<std::string>* dispatcher_);
    ~DllLoader()noexcept;

    DllLoader(const DllLoader&) =delete;
	DllLoader& operator=(const DllLoader&) =delete;

private:
    Callback<std::string, DllLoader> m_callBack;
    std::vector<void *> m_symbolHandlers;

    void Load(const std::string& dllPath_);

    void EmptyFunc();
};


} //namespace hrd28

#endif //__ILRD_HRD28_PNP_HPP__