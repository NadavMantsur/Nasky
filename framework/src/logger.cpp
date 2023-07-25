/*******************************************************************************
Project: Logger
Author:  Nadav
Reviewer: Gita
Version: 1.0 - 06/06/2023 ready for review
*******************************************************************************/
#include <fstream>      //std::fstream
#include <string>       //std::string
#include <thread>	    //std::thread
#include <functional>   //bind
#include <atomic>       //load
#include <condition_variable>

#include <ctime>        //time_t

#include "logger.hpp"

using namespace hrd28;

void Logger::Log(const std::string& msg_, LogLevel severity_)
{
    if (severity_ < m_filter)
    {
        return;
    }
    
    std::string timeStamp = GetTimestamp();
    std::string log_msg = "[" + timeStamp + "] [" + 
                          std::to_string(static_cast<int>(severity_)) +
                          "] " + msg_ + "\n";
    
    m_queue.Push(log_msg);
}

void Logger::Log(const std::string& msg_, const std::string& filename_,
                                        size_t line_, LogLevel severity_)
{
    std::string log_msg = "[" + filename_ + ":" + std::to_string(line_) + "] " + 
                          msg_ + "\n";

    Log(log_msg, severity_);
}

void Logger::WriteLog(const std::string& log_msg)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_log_fd << log_msg;
}

void Logger::SetPath(const std::string& path_)
{
    // std::lock_guard<std::mutex> lock(m_logMutex); //not support c.v
    std::unique_lock<std::mutex> lock(m_logMutex);

    // Wait for the queue to be empty.
    while (!m_queue.IsEmpty())
    {
        m_queue_cond.wait(lock);
    }

    // Set the path.
    m_log_fd.close();
    m_log_fd.open(path_, std::ios::out | std::ios::app);
}

void Logger::SetFilter(LogLevel severity_)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_filter = severity_;
}

Logger::Logger(): m_filter(LogLevel::DEBUG), 
                        m_thread(&Logger::LogThreadFunc, this), should_stop(0), m_queue_cond()
{
    m_log_fd.open("log.txt", std::ios_base::app);
}

void Logger::LogThreadFunc()
{
    while(!should_stop.load(std::memory_order_relaxed))
    {
        if(!m_queue.IsEmpty())
        {
            std::string s;
            
            m_queue.Pop(&s);
            
            m_log_fd << s.c_str();
            
            s = "";
        }
    }
}

Logger::~Logger() noexcept
{
    should_stop.store(1,std::memory_order_relaxed);
    m_thread.join();
    m_log_fd.close();
}

std::string Logger::GetTimestamp()
{
    time_t now = time(NULL);
    std::string timeStamp = ctime(&now);
    timeStamp.erase(timeStamp.length() - 1);

    return timeStamp;
}