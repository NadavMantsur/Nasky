/*******************************************************************************
Project: Framework
Author:  HRD28
Version: 1.0 - 26/06/23
         1.1 - 28/06/23 - added function AddGateway
*******************************************************************************/
#ifndef __ILRD_HRD28_FRAMEWORK_HPP__
#define __ILRD_HRD28_FRAMEWORK_HPP__

#include <string>
#include <vector>

#include <cstddef>

#include "Gateway.hpp"
#include "logger.hpp"
#include "driver_data.hpp"
#include "thread_pool.hpp"
#include "factory.hpp"
#include "pnp.hpp"

namespace hrd28
{

class IReqEngTask;

template <typename KEY, typename ARGS>
class Framework
{
public:
	explicit Framework(const std::string& dirMonitorPath_, size_t numOfThread_);
	
	~Framework() noexcept;
	
	Framework(const Framework &o_)=delete;
	Framework &operator= (const Framework& o_)=delete;
	
	void Run();
	
    using CREATE_FUNC = std::function<std::shared_ptr<IReqEngTask>(ARGS)>;
	
	void ConfigTask(KEY key_, CREATE_FUNC create_func_);
	void ConfigSetNumThreads(size_t newNum_);
	void ConfigLoggerPath(const std::string &newPath_);
	void ConfigLoggerLevel(Logger::LogLevel newLevel_);
	
	void AddGateway(IGateway<KEY, ARGS> newGateway_);
	 
	//the ~Framework will use stop, if the usr didnt use it already
	void Stop();
	
private:
	std::vector<IGateway<KEY, ARGS> > m_gw;
	
	ThreadPool m_tp; //sizeForPool
	
	//(no one should give us this - that's why its by val)
	DirMonitor m_dirMonitor; //dirMonitorPath_ 
};


template <typename KEY, typename ARGS>
Framework::Framework(const std::string& dirMonitorPath_, size_t numOfThread_)
:m_tp(numOfThread_), m_dirMonitor(dirMonitorPath_)
{}

template <typename KEY, typename ARGS>
Framework::~Framework() noexcept
{
	Framework::Stop();
}

template <typename KEY, typename ARGS>
void Framework::Run()
{
	
}


template <typename KEY, typename ARGS>
void Framework::ConfigTask(KEY key_, CREATE_FUNC create_func_)
{
	Singleton<hrd28::Factory<ITask, ActionType, Taskparams>>::GetInstance()->SetKey(key_, create_func_);
}

template <typename KEY, typename ARGS>
void Framework::ConfigSetNumThreads(size_t newNum_)
{
	m_tp.SetNumOfThreads(newNum_);
}

template <typename KEY, typename ARGS>
void Framework::ConfigLoggerPath(const std::string &newPath_)
{
	Singleton<hrd28::Logger::GetInstance()->SetPath(const std::string& newPath_);
}

template <typename KEY, typename ARGS>
void Framework::ConfigLoggerLevel(Logger::LogLevel newLevel_)
{
	Singleton<hrd28::Logger::GetInstance()->SetFilter(LogLevel newLevel_);
}

template <typename KEY, typename ARGS>
void Framework::AddGateway(IGateway<KEY, ARGS> newGateway_)
{
	m_gw.pushback(newGateway_);
}

template <typename KEY, typename ARGS>
void Framework::Stop()
{
	m_tp.Stop();
}

}//hrd28


#endif
