/*******************************************************************************
 * Project: NASky - RAM storage
 * Author:  HRD28
 * Version: 1.0 - 24/05/2023 
 *          1.1 - 25/05/2023  -  Added comments about exceptions throws.
*******************************************************************************/
#ifndef ILRD_HRD28_RAM_STORAGE_HPP
#define ILRD_HRD28_RAM_STORAGE_HPP

#include <memory>     // std::shared_ptr
#include <vector>     // std::vector
#include <cstddef>    // size_t

#include "storage.hpp"

namespace hrd28
{
class RAMStorage: public IStorage //uncopyable
{
public:
    explicit RAMStorage(size_t capacity_); //capacity in bytes
    ~RAMStorage() noexcept;

    void Read(std::shared_ptr<DriverData> data_) const override;
    void Write(std::shared_ptr<DriverData> data_) override; 

    std::vector<uint8_t> Get()
    {
        return m_storage;
    }

private:
    std::vector<uint8_t> m_storage;
};

// std::vector<uint8_t> RAMStorage::Get()
// {
// return m_storage;
// }


}//namespace hrd28

#endif //ILRD_HRD28_RAM_STORAGE_HPP