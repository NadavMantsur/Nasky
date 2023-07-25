/*******************************************************************************
 * Project: NASky - driver data
 * Author:  HRD28
 * Version: 1.0 - 29/05/2023
 *          1.1 - 04/06/2023 - made all references const 
*******************************************************************************/
#ifndef ILRD_HRD28_DRIVER_DATA_HPP
#define ILRD_HRD28_DRIVER_DATA_HPP

#include <cstdint>  //int16_t, int64_t, uint64_t, uint32_t, uint8_t
#include <vector>   //std::vector

#include "utils.hpp"

namespace hrd28
{
enum class ActionType {READ, WRITE, DISC, FLUSH, TRIM};

struct DriverData
{
    explicit DriverData(int32_t type_, int64_t msg_id_, uint64_t offset_,
                         uint32_t length_, const std::vector<uint8_t>& buffer_);
    DEBUG_ONLY(DriverData(const DriverData& o_) =default;)

    ActionType m_type;
    int64_t m_msg_id;
    const uint64_t m_offset;
    const uint32_t m_length;
    std::vector<uint8_t> m_buffer;

    static ActionType GetAction(int32_t type_); //throws DriverException
};
}//hrd28 namespace

#endif //ILRD_HRD28_DRIVER_DATA_HPP