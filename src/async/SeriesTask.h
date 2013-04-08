#pragma once
#include "async/Platform.h"
#include "async/Async.h"

#include "workers/Task.h"

#include <functional>
#include <vector>

namespace async_cpp {

namespace workers {
class IManager;
}

namespace async {

class ASYNC_API ISeriesTask : public workers::Task {
public:
    ISeriesTask(std::shared_ptr<workers::IManager> mgr, std::function<AsyncFuture(AsyncResult&)> generateResult);
    virtual ~ISeriesTask();

    virtual void cancel() = 0;

    inline void forwardFuture(AsyncFuture&& future);
protected:
    AsyncResult getResult();

    std::function<AsyncFuture(AsyncResult&)> mGenerateResultFunc;
    std::shared_ptr<workers::IManager> mManager;

private:
    bool mHasForwardedFuture;
    AsyncFuture mForwardedFuture;
};

/**
 * Task which continues a chain of asynchronous tasks.
 */
class ASYNC_API SeriesTask : public ISeriesTask {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    SeriesTask(std::shared_ptr<workers::IManager> mgr, 
        std::function<AsyncFuture(AsyncResult&)> generateResult,
        std::shared_ptr<ISeriesTask> nextTask);    
    virtual ~SeriesTask();

    virtual void cancel();

protected:
    virtual void performSpecific();

private:
    std::shared_ptr<ISeriesTask> mNextTask;
};

//------------------------------------------------------------------------------
class SeriesTerminalTask;
class ASYNC_API SeriesCollectTask : public ISeriesTask {
public:
    SeriesCollectTask(std::shared_ptr<workers::IManager> mgr, std::function<AsyncFuture(AsyncResult&)> generateResult);
    virtual ~SeriesCollectTask();

    virtual void cancel();

    inline AsyncFuture getFuture();
protected:
    virtual void performSpecific();

private:
    std::function<AsyncFuture(AsyncResult&)> mGenerateResultFunc;
    std::promise<AsyncResult> mPromise;
    std::shared_ptr<SeriesTerminalTask> mTerminalTask;
};

//------------------------------------------------------------------------------
class ASYNC_API SeriesTerminalTask : public workers::Task {
public:
    SeriesTerminalTask();
    virtual ~SeriesTerminalTask();

    void cancel();

    inline void forwardFuture(AsyncFuture&& future);
    inline AsyncFuture getFuture();

protected:
    virtual void performSpecific();

private:
    AsyncFuture mForwardedFuture;
    std::promise<AsyncResult> mPromise;
};

//inline implementations
//------------------------------------------------------------------------------
void ISeriesTask::forwardFuture(AsyncFuture&& forwardedFuture)
{
    mHasForwardedFuture = true;
    mForwardedFuture = std::move(forwardedFuture);
}

//------------------------------------------------------------------------------
AsyncFuture SeriesCollectTask::getFuture()
{
    return mTerminalTask->getFuture();
}

//------------------------------------------------------------------------------
void SeriesTerminalTask::forwardFuture(AsyncFuture&& forwardedFuture)
{
    mForwardedFuture = std::move(forwardedFuture);
}

//------------------------------------------------------------------------------
AsyncFuture SeriesTerminalTask::getFuture()
{
    return mPromise.get_future();
}

}
}