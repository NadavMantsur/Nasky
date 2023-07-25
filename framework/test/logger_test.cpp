// gp11d -iquote ~/nadav/projects/final_project/framework/include src/logger.cpp test/logger_test.cpp

#include <iostream>
#include <thread>
#include <string>

#include "logger.hpp"

using namespace hrd28;
using namespace std;

int main()
{
    // Log messages using the macros
    DLOG("This is a debug message"); // This message will not be logged because the filter level is INFO
    ILOG("This is an info message"); // This message will be logged
    WLOG("This is a warning message"); // logged
    ELOG("This is an error message"); // logged
    CLOG("This is a critical message"); // logged


    //setfilter
    Singleton<Logger>::GetInstance()->SetFilter(Logger::LogLevel::DEBUG);
    DLOG("After change filter.");
    DLOG("This is a debug message.");

    std::string initialPath = "initial.log";
    Singleton<Logger>::GetInstance()->SetPath(initialPath);
    DLOG("After change path.");
    DLOG("This is a debug message.");
    return 0;
}