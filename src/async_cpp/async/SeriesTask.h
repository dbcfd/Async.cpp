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
    SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask);    
    SeriesTask(SeriesTask&& other);
    virtual ~SeriesTask();

    virtual void cancel();

protected:
    virtual void notifyFailureToPerform() final;
    virtual void performSpecific() final;

private:
    std::shared_ptr<ISeriesTask<TDATA>> mNextTask;
    std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> mGenerateResultFunc;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)> generateResult,
        std::shared_ptr<ISeriesTask<TDATA>> nextTask)
    : ISeriesTask<TDATA>(mgr), mNextTask(nextTask), mGenerateResultFunc(generateResult)
{
    assert(mNextTask);
}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesTask<TDATA>::SeriesTask(SeriesTask&& other)
    : ISeriesTask<TDATA>(std::move(other)), mNextTask(std::move(other.mNextTask)), mGenerateResultFunc(std::move(other.mGenerateResultFunc))
{
    
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
            try
            {
                mNextTask->forwardFuture(mGenerateResultFunc(mForwardedFuture.get()));
            }
            catch(std::exception& ex)
            {
                mNextTask->forwardFuture(AsyncResult<TDATA>(ex.what()).asFulfilledFuture());
            }
            catch(...)
            {
                mNextTask->forwardFuture(AsyncResult<TDATA>("Series: Unknown exception, please verify series tasks or use std::exception").asFulfilledFuture());
            }
            if(auto mgr = mManager.lock())
            {
                mgr->run(mNextTask);
            }
            else
            {
                mNextTask->failToPerform();
            }
        }
        else
        {
            if(auto mgr = mManager.lock())
            {
                mgr->run(std::make_shared<SeriesTask<TDATA>>(std::move(*this)));
            }
            else
            {
                notifyFailureToPerform();
            }
        }
    }
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::notifyFailureToPerform()
{
    mNextTask->failToPerform();
}

}
}