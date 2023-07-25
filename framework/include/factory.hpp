/*******************************************************************************
Project: Factory
Author:  HRD28
Version: 1.0 - 05/06/2023
         1.1 - 08/06/2023 changed Add() to SetKey(), to clarify the possibility
                          to change an existing method
         1.2 - 11/06/2023 switched from unique_ptr to shared_ptr
*******************************************************************************/
#ifndef __ILRD_HRD28_FACTORY_HPP__
#define __ILRD_HRD28_FACTORY_HPP__

#include <memory>           //std::shared_ptr
#include <unordered_map>    //std::unordered_map
#include <exception>        //std::exception
#include <functional>
#include <mutex>            //std::mutex

#include "singleton.hpp"

namespace hrd28
{
class FactoryBadAlloc: public std::bad_alloc
{
public:
    explicit FactoryBadAlloc() =default;
    FactoryBadAlloc(const FactoryBadAlloc&) =default;
};

class FactoryBadKey: public std::invalid_argument
{
public:
    explicit FactoryBadKey(const std::string& what_):
                                                std::invalid_argument(what_) {}
    FactoryBadKey(const FactoryBadKey&) =default;
};

class FactoryInvalidArguments: public std::invalid_argument
{
public:
    explicit FactoryInvalidArguments(const std::string& what_):
                                                std::invalid_argument(what_) {}
    FactoryInvalidArguments(const FactoryInvalidArguments&) =default;
};

template <typename BASE, typename KEY, typename ARGS>
class Factory
{
public:
    Factory(const Factory&) =delete;
    Factory& operator=(const Factory&) =delete;

    using CREATE_FUNC = std::function<std::shared_ptr<BASE>(ARGS)>;
    void SetKey(const KEY& key_, CREATE_FUNC create_func_);
    std::shared_ptr<BASE> Create(const KEY& key_, ARGS args_) const;

private:
    explicit Factory() =default;
    ~Factory() noexcept =default;

    friend Singleton<Factory>; //Factory<BASE...>

    std::unordered_map<KEY, CREATE_FUNC> m_classes;

    mutable std::mutex m_factory_guard;
};

////////////////////////////////////////////////////////////////////////////////

template <typename BASE, typename KEY, typename ARGS>
void Factory<BASE, KEY, ARGS>::SetKey(const KEY& key_, CREATE_FUNC create_func_)
{
  std::lock_guard<std::mutex> lock(m_factory_guard);

  m_classes[key_] = create_func_;
}

// function to create an instance of the specified key.
// if the key does not exists, throws FactoryBadKey.
// If a std::bad_alloc or std::invalid_argument exception is caught during
// object creation, it rethrows the FactoryBadAlloc, FactoryInvalidArguments
// respectively.
template <typename BASE, typename KEY, typename ARGS>
std::shared_ptr<BASE> Factory<BASE, KEY, ARGS>::Create(const KEY& key_, ARGS args_) const
{
    std::lock_guard<std::mutex> lock(m_factory_guard);

    auto it = m_classes.find(key_);

    // Key not found in the factory
    if (it == m_classes.end())
    {
        throw FactoryBadKey("invalid key");
    }

    try
    {
        // Invoke the create function with the provided arguments
        return it->second(std::forward<ARGS>(args_));
    }
    catch (const std::bad_alloc&)
    {
        throw FactoryBadAlloc();
    }
    catch (const std::invalid_argument&)
    {
        throw FactoryInvalidArguments("invalid argument");
    }
    catch (...)
    {
        throw;
    }
}

}//namespace hrd28

#endif //__ILRD_HRD28_FACTORY_HPP__