#pragma once
#include "async_cpp/async/detail/ISeriesTask.h"

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Task which continues a chain of asynchronous tasks.
 */
template<class TDATA>
class SeriesTask : public ISeriesTask<TDATA> {
public:
    typedef typename std::function<void(OpResult<TDATA>&&)> callback_t;
    typedef typename std::function<void(OpResult<TDATA>&&, typename callback_t)> operation_t;
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask);    
    virtual ~SeriesTask();

protected:
    virtual void notifyCancel() final;
    virtual void notifyException(const std::exception& ex) final;
    virtual void performSpecific() final;

private:
    std::shared_ptr<ISeriesTask<TDATA>> mNextTask;
    typename operation_t mGenerateResultFunc;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask)
    : ISeriesTask<TDATA>(mgr), mNextTask(nextTask), mGenerateResultFunc(generateResult)
{
    if(!mNextTask) { throw(std::runtime_error("SeriesTask: No next task")); }
}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::~SeriesTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::performSpecific()
{
    auto callback = [this](OpResult<TDATA>&& result)->void {
        mNextTask->begin(std::move(result));
    };

    mGenerateResultFunc(std::move(mPreviousResult), callback);
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::notifyCancel()
{
    mNextTask->begin(OpResult<TDATA>(std::string("Cancelled")));
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::notifyException(const std::exception& ex)
{
    mNextTask->begin(OpResult<TDATA>(std::string(ex.what())));
}

}
}
}