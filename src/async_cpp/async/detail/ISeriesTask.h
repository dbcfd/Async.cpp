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

    typename VariantType mPreviousResult;
};

//inline implementations
//------------------------------------------------------------------------------
template<class T>
ISeriesTask<T>::ISeriesTask(std::weak_ptr<tasks::IManager> mgr)
    : IAsyncTask(mgr)
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
    mManager.lock()->run(shared_from_this());
}

}
}
}