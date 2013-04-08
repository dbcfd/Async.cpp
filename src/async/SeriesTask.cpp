#include "async/SeriesTask.h"
#include "async/AsyncResult.h"
#include "workers/BasicTask.h"
#include "workers/IManager.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ISeriesTask::ISeriesTask(std::shared_ptr<workers::IManager> mgr, std::function<AsyncFuture(AsyncResult&)> generateResult)
    : mManager(mgr), mGenerateResultFunc(generateResult), mHasForwardedFuture(false)
{
    assert(mgr);
    mWasRun = false;
}

//------------------------------------------------------------------------------
ISeriesTask::~ISeriesTask()
{
    
}

//------------------------------------------------------------------------------
AsyncResult ISeriesTask::getResult()
{
    AsyncResult result;
    if(mHasForwardedFuture)
    {
        result = mForwardedFuture.get();
    }
    return result;
}

//------------------------------------------------------------------------------
SeriesTask::SeriesTask(std::shared_ptr<workers::IManager> mgr, 
        std::function<AsyncFuture(AsyncResult&)> generateResult,
        std::shared_ptr<ISeriesTask> nextTask)
    : ISeriesTask(mgr, generateResult), mNextTask(nextTask)
{
    assert(mNextTask);
}

//------------------------------------------------------------------------------
SeriesTask::~SeriesTask()
{
    if(!mWasRun)
    {
        cancel();
    }
}

//------------------------------------------------------------------------------
void SeriesTask::cancel()
{
    mNextTask->cancel();
}

//------------------------------------------------------------------------------
void SeriesTask::performSpecific()
{
    mWasRun.exchange(true);
    AsyncFuture futureToForward;
    try {
        futureToForward = mGenerateResultFunc(getResult());
    }
    catch(std::exception& ex)
    {
        futureToForward = AsyncResult(ex.what()).asFulfilledFuture();
    }
    mNextTask->forwardFuture(std::move(futureToForward));
    mManager->run(mNextTask);
}

//------------------------------------------------------------------------------
SeriesCollectTask::SeriesCollectTask(std::shared_ptr<workers::IManager> mgr,
                                       std::function<AsyncFuture(AsyncResult&)> generateResult)
                                       : ISeriesTask(mgr, generateResult), mTerminalTask(new SeriesTerminalTask())
{

}

//------------------------------------------------------------------------------
SeriesCollectTask::~SeriesCollectTask()
{
    if(!mWasRun)
    {
        cancel();
    }
}

//------------------------------------------------------------------------------
void SeriesCollectTask::cancel()
{
    mTerminalTask->cancel();
}

//------------------------------------------------------------------------------
void SeriesCollectTask::performSpecific()
{
    mWasRun.exchange(true);
    AsyncFuture futureToForward;
    try {
        futureToForward = mGenerateResultFunc(getResult());
    }
    catch(std::exception& ex)
    {
        futureToForward = AsyncResult(ex.what()).asFulfilledFuture();
    }
    mTerminalTask->forwardFuture(std::move(futureToForward));
    mManager->run(mTerminalTask);
}

//------------------------------------------------------------------------------
SeriesTerminalTask::SeriesTerminalTask()
{
    mWasRun = false;
}

//------------------------------------------------------------------------------
SeriesTerminalTask::~SeriesTerminalTask()
{
    if(!mWasRun)
    {
        cancel();
    }
}

//------------------------------------------------------------------------------
void SeriesTerminalTask::cancel()
{
    try {
        mPromise.set_value(AsyncResult("Cancelled Task"));
    }
    catch(std::future_error&)
    {
        //task has complete successfully
    }
}

//------------------------------------------------------------------------------
void SeriesTerminalTask::performSpecific()
{
    mWasRun.exchange(true);
    mPromise.set_value(mForwardedFuture.get());
}

}
}
