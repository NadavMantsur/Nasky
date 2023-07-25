/*******************************************************************************
Project: IGateway
Author:  HRD28
Version: 1.0 - 26/06/23
		 1.1 - 28/06/23 - added field m_driverCom in struct Taskparams
*******************************************************************************/
#ifndef __ILRD_HRD28_GATEWAY_HPP__
#define __ILRD_HRD28_GATEWAY_HPP__

#include <memory>

#include "ram_storage.hpp"
#include "thread_pool.hpp"
#include "driver_data.hpp"
#include "nbd_driver.hpp"

namespace hrd28
{

template <typename KEY, typename PARAMS>
class IGateway
{
public:
	explicit IGateway();
	virtual ~IGateway() noexcept;
	
	IGateway(const IGateway& o_) =delete;
	IGateway& operator=(const IGateway& o_) =delete;
	
	virtual int GetFd() const=0;
	
	virtual std::pair<KEY,PARAMS> Read()=0;
	
	//IDriverCom will be with MasterGateWay
};

struct Taskparams
{
    using Priority = hrd28::ITask::Priority;
    Priority m_priority;
    std::shared_ptr<RAMStorage> m_storage;
    std::shared_ptr<DriverData> m_request;
    std::shared_ptr<NBDDriverCom> m_driverCom;
};

template <typename KEY, typename PARAMS>
IGateway::IGateway()
{}

template <typename KEY, typename PARAMS>
virtual IGateway::~IGateway() noexcept
{}


}//hrd28

#endif //__ILRD_HRD28_GATEWAY_HPP__
