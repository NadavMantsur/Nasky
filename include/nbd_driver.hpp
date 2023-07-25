/*******************************************************************************
 * Project: NASky - NBD driver
 * Author:  HRD28
 * Version: 1.0 - 24/05/2023  
 *          1.1 - 25/05/2023  -  Added comments about exceptions throws.
 *                            -  DriverData Struct updated
 *                            -  enum added to the NBDDriverCom class private
 *          1.2 - 25/05/2023  -  Added comments in Ctor about exceptions throws.
 *          1.3 - 26/05/2023  -  Add comments in disconnect method
 *          1.4 - 26/05/2023  -  Change the type name of m_device_fd member 
 *          1.5 - 28/05/2023  -  added definition of FDGuard
 *          1.6 - 04/06/2023  -  made all references const
*******************************************************************************/
#ifndef ILRD_HRD28_NBD_DRIVER_HPP
#define ILRD_HRD28_NBD_DRIVER_HPP

#include <thread>   //std::thread
#include <vector>   //std::vector
#include <string>   //std::string
#include <mutex>

#include "driver_communicator.hpp"
#include "driver_data.hpp"

namespace hrd28
{
//nothing to see here, continue reading after namespace internal
namespace internal
{
class FDGuard
{
public:
    explicit FDGuard(const std::string& dev_path_); //throws DriverException
    ~FDGuard() noexcept; 

    int GetFD() const;

    void CloseFD(); //throws DriverException

private:
    int m_fd;
};
}//namespace internal

class NBDDriverCom: public IDriverCommunicator //uncopyable
{
public:
    explicit NBDDriverCom(const std::string& dev_path_, size_t storage_capacity_); //throws DriverException 
    ~NBDDriverCom() noexcept;
    
    virtual std::shared_ptr<DriverData> RecvRequest() const override; //throws DriverException
    virtual void SendReply(std::shared_ptr<DriverData> reply_) const override; //throws DriverException

    virtual int GetFD() const noexcept override;

    virtual void Disconnect() override; //throws DriverException

private:
    enum {APP = 0, DEVICE = 1, NUM_OF_SOCKETS};

    int m_sockets[NUM_OF_SOCKETS];
    internal::FDGuard m_device_fd;
    std::thread m_thread_id;
    bool m_is_connected;

    mutable std::mutex m_mutex;
    
    void ThreadRoutine();
};
}//namespace hrd28

#endif  //ILRD_HRD28_NBD_DRIVER_HPP