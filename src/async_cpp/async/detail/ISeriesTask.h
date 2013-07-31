#pragma once
#include "async_cpp/async/detail/IAsyncTask.h"
#include "async_cpp/tasks/IManager.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
template<class T>
class ISeriesTask : public IAsyncTask<T> {
public:
    ISeriesTask(std::weak_ptr<tasks::IManager> mgr);
    virtual ~ISeriesTask();

    void begin(typename VariantType&& result);

protected:
    ISeriesTask(const ISeriesTask& other);

    virtual void notifyCancel() final;
    virtual void notifyException(std::exception_ptr ex) final;

    typename VariantType mPreviousResult;
    bool mValid;
};

//inline implementations
//------------------------------------------------------------------------------
template<class T>
ISeriesTask<T>::ISeriesTask(std::weak_ptr<tasks::IManager> mgr)
    : IAsyncTask(mgr), mValid(true)
{
    
}

//------------------------------------------------------------------------------
template<class T>
ISeriesTask<T>::~ISeriesTask()
{
    
}

//------------------------------------------------------------------------------
template<class T>
void ISeriesTask<T>::begin(typename VariantType&& result)
{
    mPreviousResult = std::move(result);
    if(mValid)
    {
        mManager.lock()->run(shared_from_this());
    }
    else
    {
        cancel();
    }
}

//------------------------------------------------------------------------------
template<class T>
void ISeriesTask<T>::notifyCancel()
{
    mValid = false;
    attemptOperation([]()->void {
        throw(std::runtime_error("Cancelled"));
    } );
}

//------------------------------------------------------------------------------
template<class T>
void ISeriesTask<T>::notifyException(std::exception_ptr ex)
{
    mValid = false;
    attemptOperation([ex]()->void {
        std::rethrow_exception(ex);
    } );
}

}
}
}