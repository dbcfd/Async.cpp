#pragma once
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/IManager.h"
#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace async {

template<class TDATA>
class ISeriesTask : public tasks::Task {
public:
    ISeriesTask(std::weak_ptr<tasks::IManager> mgr);
    ISeriesTask(ISeriesTask&& other);
    virtual ~ISeriesTask();

    virtual void cancel() = 0;

    void forwardFuture(std::future<AsyncResult<TDATA>>&& future);
protected:
    ISeriesTask(const ISeriesTask& other);

    bool mIsCancelled;
    std::weak_ptr<tasks::IManager> mManager;
    std::future<AsyncResult<TDATA>> mForwardedFuture;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::ISeriesTask(std::weak_ptr<tasks::IManager> mgr)
    : Task(), mManager(mgr), mIsCancelled(false)
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::ISeriesTask(ISeriesTask&& other)
    : Task(), mManager(other. mManager), mForwardedFuture(std::move(other.mForwardedFuture)), mIsCancelled(other.mIsCancelled)
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::~ISeriesTask()
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
void ISeriesTask<TDATA>::forwardFuture(std::future<AsyncResult<TDATA>>&& forwardedFuture)
{
    mForwardedFuture = std::move(forwardedFuture);
}

}
}