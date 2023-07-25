/*******************************************************************************
 * Project: NASky - utils
 * Author:  HRD28
 * Version: 1.0 - 24/05/2023 
 *          1.1 - 25/05/2023 - update the DriverFd ctor params  
 *                           - Add comments about exceptions throws.
 *          1.2 - 26/05/2023 - Add CloseFd function in DriverFd
 *          1.3 - 26/05/2023 - Change DriverFd class name to FDGuard
 *          1.4 - 28/05/2023 - moved FDGuard to driver_communicator.hpp
*******************************************************************************/
#ifndef ILRD_HRD28_UTILS_HPP
#define ILRD_HRD28_UTILS_HPP

#ifdef NDEBUG
#define DEBUG_ONLY(...)
#else
#define DEBUG_ONLY(...) __VA_ARGS__
#endif


namespace hrd28
{
class Uncopyable
{
public:
    explicit Uncopyable() =default;
    
    Uncopyable(const Uncopyable& o_) =delete;
    Uncopyable& operator=(const Uncopyable& o_) =delete;
};



}//hrd28

#endif //ILRD_HRD28_UTILS_HPP