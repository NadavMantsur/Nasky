/*******************************************************************************
Project: Logger
Author:  HRD28
Version: 1.0 - 01/06/2023
		 1.1 - 01/06/2023 - scoped enumeration
		 					struct Message  
		 					Log overload with templates
		 					macros with templates of Log
         1.2 - 04/06/2023 - changed order of severity in LogLevel
                            overloading of Log()
                            SetPath() and SetFilter() 
*******************************************************************************/
#ifndef __ILRD_HRD28_LOGGER_HPP__
#define __ILRD_HRD28_LOGGER_HPP__

#include <fstream>      //std::fstream
#include <string>       //std::string
#include <thread>	    //std::thread
#include <functional>   //bind
#include <atomic>       //load
#include <condition_variable>


#include "singleton.hpp"
#include "waitable_queue.hpp"

namespace hrd28
{
#define DLOG(msg) (Singleton<Logger>::GetInstance()->Log((msg), __FILE__, __LINE__, Logger::LogLevel::DEBUG))
#define ILOG(msg) (Singleton<Logger>::GetInstance()->Log((msg), __FILE__, __LINE__, Logger::LogLevel::INFO))
#define WLOG(msg) (Singleton<Logger>::GetInstance()->Log((msg), __FILE__, __LINE__, Logger::LogLevel::WARNING))
#define ELOG(msg) (Singleton<Logger>::GetInstance()->Log((msg), __FILE__, __LINE__, Logger::LogLevel::ERROR))
#define CLOG(msg) (Singleton<Logger>::GetInstance()->Log((msg), __FILE__, __LINE__, Logger::LogLevel::CRITICAL))

class Logger
{
public:
    Logger(const Logger& o_) = delete;
    Logger& operator=(const Logger& o_) = delete;
    
	enum class LogLevel 
	{
		DEBUG = 0,
		INFO = 1,
		WARNING = 2,
		ERROR = 3,
		CRITICAL = 4
	};
    
	void Log(const std::string& msg_, LogLevel severity_);
	void Log(const std::string& msg_, const std::string& filename_,
											size_t line_, LogLevel severity_);

    void SetPath(const std::string& path_);
    void SetFilter(LogLevel severity_);
    
private:
    friend Singleton<Logger>;
    
    explicit Logger();
    ~Logger() noexcept;

    void LogThreadFunc();
    void WriteLog(const std::string& log_msg);
    std::string GetTimestamp();

	LogLevel m_filter;
	std::thread m_thread;
    std::fstream m_log_fd;
    
    WaitableQueue<std::queue<std::string>> m_queue;

    std::mutex m_logMutex; // Mutex for thread-safety 

    std::atomic<int> should_stop;

    std::condition_variable m_queue_cond;
};

} // namespace hrd28

#endif // __ILRD_HRD28_LOGGER_HPP__