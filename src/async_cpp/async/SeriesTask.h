#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ISeriesTask.h"

namespace async_cpp {
namespace async {

/**
 * Task which continues a chain of asynchronous tasks.
 */
template<class TDATA>
class SeriesTask : public ISeriesTask<TDATA> {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    SeriesTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask);    
    virtual ~SeriesTask();

    virtual void cancel();

protected:
    virtual void performSpecific();

private:
    std::shared_ptr<ISeriesTask<TDATA>> mNextTask;
    std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> mGenerateResultFunc;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::SeriesTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask)
    : ISeriesTask<TDATA>(mgr), mNextTask(nextTask), mGenerateResultFunc(generateResult)
{
    assert(mNextTask);
}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::~SeriesTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::cancel()
{
    mIsCancelled = true;
    mNextTask->cancel();
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::performSpecific()
{
    if(!mIsCancelled)
    {
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
        if(mForwardedFuture._Is_ready())
#else
        if(std::future_status::ready == mForwardedFuture.wait_for(std::chrono::milliseconds(0)))
#endif
        {
            mNextTask->forwardFuture(mGenerateResultFunc(mForwardedFuture.get()));
            mManager->run(mNextTask);
        }
        else
        {
            reset();
            mManager->run(shared_from_this());
        }
    }
}


}
}