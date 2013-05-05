#pragma once
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/IManager.h"
#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace async {

template<class TDATA>
class ISeriesTask : public tasks::Task {
public:
    ISeriesTask(std::shared_ptr<tasks::IManager> mgr);
    virtual ~ISeriesTask();

    virtual void cancel() = 0;

    void forwardFuture(std::future<AsyncResult<TDATA>>&& future);
protected:
    bool mIsCancelled;
    std::shared_ptr<tasks::IManager> mManager;
    std::future<AsyncResult<TDATA>> mForwardedFuture;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::ISeriesTask(std::shared_ptr<tasks::IManager> mgr)
    : mManager(mgr), mIsCancelled(false)
{
    assert(mgr);
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