#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/SeriesTerminalTask.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
class SeriesCollectTask : public ISeriesTask<TDATA> {
public:
    SeriesCollectTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> generateResult);
    SeriesCollectTask(SeriesCollectTask&& other);
    virtual ~SeriesCollectTask();

    virtual void cancel();

    std::future<AsyncResult<TRESULT>> getFuture();
protected:
    virtual void performSpecific();
    virtual void notifyFailureToPerform();

private:
    std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> mGenerateResultFunc;
    std::shared_ptr<SeriesTerminalTask<TRESULT>> mTerminalTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
SeriesCollectTask<TDATA, TRESULT>::SeriesCollectTask(std::shared_ptr<tasks::IManager> mgr,
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
                                     mTerminalTask(std::move(mTerminalTask))
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
            std::future<AsyncResult<TRESULT>> future;
            try
            {
                future = mGenerateResultFunc(mForwardedFuture.get());
            }
            catch(std::runtime_error& ex)
            {
                future = AsyncResult<TRESULT>(ex.what()).asFulfilledFuture();
            }
            mTerminalTask->forwardFuture(std::move(future));
            mManager->run(mTerminalTask);
        }
        else
        {
            mManager->run(std::make_shared<SeriesCollectTask<TDATA, TRESULT>>(std::move(*this)));
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