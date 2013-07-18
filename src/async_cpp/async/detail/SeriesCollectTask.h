#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/detail/SeriesTerminalTask.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
class SeriesCollectTask : public ISeriesTask<TDATA> {
public:
    SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> generateResult);
    SeriesCollectTask(SeriesCollectTask&& other);
    virtual ~SeriesCollectTask();

    virtual void cancel();

    std::future<AsyncResult<TRESULT>> getFuture();
protected:
    virtual void performSpecific() final;
    virtual void notifyFailureToPerform() final;

private:
    std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> mGenerateResultFunc;
    std::shared_ptr<SeriesTerminalTask<TRESULT>> mTerminalTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
SeriesCollectTask<TDATA, TRESULT>::SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr,
                                     std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> generateResult)
                                     : ISeriesTask<TDATA>(mgr), 
                                     mGenerateResultFunc(generateResult),
                                     mTerminalTask(std::make_shared<SeriesTerminalTask<TRESULT>>(mgr))
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
SeriesCollectTask<TDATA, TRESULT>::SeriesCollectTask(SeriesCollectTask&& other)
                                     : ISeriesTask<TDATA>(std::move(other)), 
                                     mGenerateResultFunc(std::move(other.mGenerateResultFunc)),
                                     mTerminalTask(std::move(other.mTerminalTask))
{

}


//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
SeriesCollectTask<TDATA, TRESULT>::~SeriesCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void SeriesCollectTask<TDATA, TRESULT>::cancel()
{
    mTerminalTask->cancel();
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void SeriesCollectTask<TDATA, TRESULT>::performSpecific()
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
                mTerminalTask->forwardFuture(mGenerateResultFunc(mForwardedFuture.get()));
            }
            catch(std::exception& ex)
            {
                mTerminalTask->forwardFuture(AsyncResult<TRESULT>(ex.what()).asFulfilledFuture());
            }
            catch(...)
            {
                mTerminalTask->forwardFuture(AsyncResult<TRESULT>("Series: Unknown exception, please verify series tasks or use std::exception").asFulfilledFuture());
            }
            if(auto mgr = mManager.lock())
            {
                mgr->run(mTerminalTask);
            }
            else
            {
                mTerminalTask->failToPerform();
            }
        }
        else
        {
            if(auto mgr = mManager.lock())
            {
                mgr->run(std::make_shared<SeriesCollectTask<TDATA, TRESULT>>(std::move(*this)));
            }
            else
            {
                notifyFailureToPerform();
            }
        }
    }
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> SeriesCollectTask<TDATA, TRESULT>::getFuture()
{
    return mTerminalTask->getFuture();
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void SeriesCollectTask<TDATA, TRESULT>::notifyFailureToPerform()
{
    mTerminalTask->failToPerform();
}

}
}
}