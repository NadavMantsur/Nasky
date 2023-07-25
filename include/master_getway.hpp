#ifndef __ILRD_HRD28_MASTER_GATEWAY__
#define __ILRD_HRD28_MASTER_GATEWAY__

#include <memory>

#include "ram_storage.hpp"
#include "thread_pool.hpp"
#include "driver_data.hpp"
#include "nbd_driver.hpp"

#include "gateway.hpp"

namespace hrd28
{

template <typename KEY, typename PARAMS>
class Gateway: public IGateway
{
public:
	explicit Gateway();
	~Gateway() noexcept;
	
	Gateway(const Gateway& o_) =delete;
	Gateway& operator=(const Gateway& o_) =delete;
	
	int GetFd() const;
	
	std::pair<KEY,PARAMS> Read();
	
	//IDriverCom will be with MasterGateWay
private:
    Taskparams m_params;

};

template <typename KEY, typename PARAMS>
int Gateway::GetFd() const
{
    return m_params.m_driverCom.GetFD();
}
	
template <typename KEY, typename PARAMS>
std::pair<KEY,PARAMS> Gateway::Read()
{
    
}

}//hrd28
#endif //__ILRD_HRD28_MASTER_GATEWAY__