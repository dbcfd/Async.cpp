#pragma once
#include "async_cpp/async/OpResult.h"

#include "async_cpp/tasks/IManager.h"
#include "async_cpp/tasks/Task.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
template<class TDATA>
class ISeriesTask : public tasks::Task {
public:
    ISeriesTask(std::weak_ptr<tasks::IManager> mgr);
    virtual ~ISeriesTask();

    void begin(OpResult<TDATA>&& result);

protected:
    ISeriesTask(const ISeriesTask& other);

    std::weak_ptr<tasks::IManager> mManager;
    OpResult<TDATA> mPreviousResult;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::ISeriesTask(std::weak_ptr<tasks::IManager> mgr)
    : Task(), mManager(mgr)
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
ISeriesTask<TDATA>::~ISeriesTask()
{
    
}

//------------------------------------------------------------------------------
template<class TDATA>
void ISeriesTask<TDATA>::begin(OpResult<TDATA>&& result)
{
    mPreviousResult = std::move(result);
    mManager.lock()->run(shared_from_this());
}

}
}
}