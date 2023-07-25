/*******************************************************************************
 * Project: NASky - driver communicator
 * Author:  HRD28
 * Version: 1.0 - 24/05/2023   
 *          1.1 - 28/05/2023 - added definitions of ActionType and DriverData   
 *          1.2 - 29/05/2023 - change the type of "type" parameter in DriverData
 *          1.3 - 29/05/2023 - transfer driver data struct to driver_data.hpp
*******************************************************************************/
#ifndef ILRD_HRD28_DRIVER_COMMUNICATOR_HPP
#define ILRD_HRD28_DRIVER_COMMUNICATOR_HPP

#include <memory>   //std::shared_ptr
#include <system_error>

#include "utils.hpp"            //Uncopyable
#include "driver_data.hpp"     //DriverData

namespace hrd28
{

class IDriverCommunicator: private Uncopyable
{//each driver can throw exception of type DriverException
public:
    virtual ~IDriverCommunicator() noexcept =default;

    virtual std::shared_ptr<DriverData> RecvRequest() const =0; //returns reply_
    virtual void SendReply(std::shared_ptr<DriverData> reply_) const =0;

    virtual int GetFD() const =0;

    virtual void Disconnect() =0;
};

class DriverException: public std::system_error
{
public:
    explicit DriverException(std::error_code error_, const std::string& what_);
};

}//namespace hrd28

#endif //ILRD_HRD28_DRIVER_COMMUNICATOR_HPP