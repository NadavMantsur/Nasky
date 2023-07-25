/*******************************************************************************
 * Project: NASky - storage
 * Author:  HRD28
 * Version: 1.0 - 24/05/2023    
 *          1.1 - 25/05/2023 - Add "#include <cstdint>  //uint8_t"   
 *          1.2 - 28/05/2023 - Remove "#include <cstdint>  //uint8_t"   
*******************************************************************************/
#ifndef ILRD_HRD28_STORAGE_HPP
#define ILRD_HRD28_STORAGE_HPP

#include <memory>     //std::shared_ptr

#include "utils.hpp"  //Uncopyable
#include "driver_communicator.hpp" //DriverData

namespace hrd28
{
class IStorage: private Uncopyable
{
public:
    virtual ~IStorage() noexcept =default;

    virtual void Read(std::shared_ptr<DriverData> data_) const =0;
    virtual void Write(std::shared_ptr<DriverData> data_) =0;
};
}//namespace hrd28

#endif //ILRD_HRD28_STORAGE_HPP