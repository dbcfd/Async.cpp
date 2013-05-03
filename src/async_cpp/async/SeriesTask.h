#pragma once
#include "async_cpp/async/Platform.h"
#include "async_cpp/async/Async.h"

#include "async_cpp/tasks/Task.h"

#include <functional>
#include <vector>

namespace async_cpp {

namespace tasks {
class IManager;
}

namespace async {

class ASYNC_CPP_ASYNC_API ISeriesTask : public tasks::Task {
public:
    ISeriesTask(std::shared_ptr<tasks::IManager> mgr, std::function<AsyncFuture(AsyncResult&)> generateResult);
    virtual ~ISeriesTask();

    virtual void cancel() = 0;

    inline void forwardFuture(AsyncFuture&& future);
protected:
    AsyncResult getResult();

    std::function<AsyncFuture(AsyncResult&)> mGenerateResultFunc;
    std::shared_ptr<tasks::IManager> mManager;
    std::atomic_bool mWasRun;

private:
    bool mHasForwardedFuture;
    AsyncFuture mForwardedFuture;
};

/**
 * Task which continues a chain of asynchronous tasks.
 */
class ASYNC_CPP_ASYNC_API SeriesTask : public ISeriesTask {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    SeriesTask(std::shared_ptr<tasks::IManager> mgr, 
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
class ASYNC_CPP_ASYNC_API SeriesCollectTask : public ISeriesTask {
public:
    SeriesCollectTask(std::shared_ptr<tasks::IManager> mgr, std::function<AsyncFuture(AsyncResult&)> generateResult);
    virtual ~SeriesCollectTask();

    virtual void cancel();

    inline AsyncFuture getFuture();
protected:
    virtual void performSpecific();

private:
    std::promise<AsyncResult> mPromise;
    std::shared_ptr<SeriesTerminalTask> mTerminalTask;
};

//------------------------------------------------------------------------------
class ASYNC_CPP_ASYNC_API SeriesTerminalTask : public tasks::Task {
public:
    SeriesTerminalTask();
    virtual ~SeriesTerminalTask();

    void cancel();
    AsyncFuture getFuture();

    inline void forwardFuture(AsyncFuture&& future);

protected:
    virtual void performSpecific();

private:
    std::atomic_bool mWasRun;
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

}
}