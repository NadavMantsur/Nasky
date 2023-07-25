/*******************************************************************************
Project: Dispatcher Callback
Author:  HRD28
Version: 1.0 - 18/06/2023
         1.1 - add SubscriptionError class
		 1.2 - add defaule empty func in Callback ctor
		       add subscriber_ var to Callback ctor
		 1.3 - remove defaule from Icallback dtor
		 1.4 - 20/06/2023 convert weak ptr to raw ptr
		 1.5 - ~Callback =default
*******************************************************************************/
#ifndef __ILRD_HRD28_DISPATCHER_CALLBACK_HPP__
#define __ILRD_HRD28_DISPATCHER_CALLBACK_HPP__

#include <memory>		//std::weak_ptr
#include <vector>		//std::vector
#include <iterator>		//std::iterator
#include <exception>	//std::bad_alloc
#include <algorithm>	//std::find


namespace hrd28
{
template <typename MSG>
class Dispatcher;

template <typename MSG>
class ICallback;

class SubscriptionError: public std::bad_alloc
{
public:
    explicit SubscriptionError() =default;
    ~SubscriptionError() =default;
};

//ICallback<MSG> must be moveable
template <typename MSG>
class Dispatcher
{
public:
	explicit Dispatcher() =default;
	~Dispatcher() noexcept;

	Dispatcher(const Dispatcher&) =delete;
	Dispatcher& operator=(const Dispatcher&) =delete;
	
	
	void NotifyAll(const MSG& msg_);

private:
	friend ICallback<MSG>;
	using Subscribers = std::vector<ICallback<MSG>*>;

	Subscribers m_subscribers;

	//throws SubscriptionError
	void Subscribe(ICallback<MSG>* callback_);
	void Unsubscribe(ICallback<MSG>* callback_);
};


template <typename MSG>
class ICallback
{
public:
	explicit ICallback(Dispatcher<MSG>* dispatcher_);
	virtual ~ICallback() noexcept;

	ICallback(const ICallback&) =delete;
	ICallback& operator=(const ICallback&) =delete;

private:
	friend Dispatcher<MSG>;

	virtual void Notify(const MSG& msg_) =0;
	virtual void OutOfService() =0;

	Dispatcher<MSG>* m_dispatcher;
};


template <typename MSG, typename SUBS>
class Callback : public ICallback<MSG>
{
public:
	using MethodNotify = void (SUBS::*) (const MSG& msg_);
	using MethodEnd = void (SUBS::*) ();
	void EmptyFunc();

	explicit Callback(SUBS& subscriber_,
					  Dispatcher<MSG>* dispacher_,
					  MethodNotify notifyMethod_,
					  MethodEnd serviceEnded_ = EmptyFunc);

	virtual ~Callback() noexcept =default;

	Callback(const Callback&) =delete;
	Callback& operator=(const Callback&) =delete;

private:
	virtual void Notify(const MSG& msg_) override;
	virtual void OutOfService() override;

	SUBS& m_subscriber;

	MethodNotify m_notifyMethod;
	MethodEnd m_serviceEnded;


};


////////////////////////////////////////////////////////////////////////////////
template <typename MSG>
Dispatcher<MSG>::~Dispatcher() noexcept
{
	for_each(m_subscribers.begin(), m_subscribers.end(), [&](auto iteratir_)
					{ {iteratir_->OutOfService();}});
}

template <typename MSG>
void Dispatcher<MSG>::NotifyAll(const MSG& msg_)
{
	std::vector<ICallback<MSG>*> copySubscribeVector = m_subscribers;

	for (ICallback<MSG>* callback : copySubscribeVector)
	{
		callback->Notify(msg_);
	}
}

template <typename MSG>
void Dispatcher<MSG>::Subscribe(ICallback<MSG>* callback_)
{
	try
	{
		m_subscribers.push_back(callback_);
	}
	catch (const std::bad_alloc &e)
	{
		SubscriptionError err;

		throw err;
	}
}

template <typename MSG>
void Dispatcher<MSG>::Unsubscribe(ICallback<MSG>* callback_)
{
	auto iter = std::find(m_subscribers.begin(), m_subscribers.end(), callback_);

	m_subscribers.erase(iter);
}

////////////////////////////////////////////////////////////////////////////////
template <typename MSG>
ICallback<MSG>::ICallback(Dispatcher<MSG>* dispatcher_) 
: m_dispatcher(dispatcher_) 
{
	m_dispatcher->Subscribe(this);
}

template <typename MSG>
ICallback<MSG>::~ICallback() noexcept
{
	m_dispatcher->Unsubscribe(this);
}

////////////////////////////////////////////////////////////////////////////////
template <typename MSG, typename SUBS>
void Callback<MSG, SUBS>::EmptyFunc()
{}

template <typename MSG, typename SUBS>
Callback<MSG, SUBS>::Callback(SUBS& subscriber_,
									Dispatcher<MSG>* dispatcher_, 
									MethodNotify notifyMethod_,
									MethodEnd serviceEnded_):
													ICallback<MSG>(dispatcher_),
													m_subscriber(subscriber_),
													m_notifyMethod(notifyMethod_),
													m_serviceEnded(serviceEnded_)
{}

template <typename MSG, typename SUBS>
void Callback<MSG, SUBS>::Notify(const MSG& msg_)
{
	(m_subscriber.*m_notifyMethod)(msg_);
}

template <typename MSG, typename SUBS>
void Callback<MSG, SUBS>::OutOfService()
{
	(m_subscriber.*m_serviceEnded)();
}

} //namespace hrd28

#endif //__ILRD_HRD28_DISPATCHER_CALLBACK_HPP__
