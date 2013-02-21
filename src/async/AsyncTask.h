#pragma once
#include "async/Platform.h"
#include "async/Async.h"
#include "async/AsyncResult.h"

#include "workers/Task.h"

#include <functional>

namespace async_cpp {
namespace async {

/**
 * Implementation of IAsyncTask that does not receive forwarded results from other tasks.
 */
class ASYNC_API AsyncTask : public workers::Task {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    AsyncTask(std::function<void(void)> generateResult);

protected:
    virtual void performSpecific();

private:
    std::function<void(void)> mGenerateResultFunc;
};

/**
 * Asynchronous task interface. Provides task interface and methodology to retrieve future
 */
class ASYNC_API IForwardingTask : public workers::Task {
public:
    IForwardingTask();
    virtual ~IForwardingTask();

    /**
     * Forward the result of a previous task to this task.
     * @param result Result to forward
     */
    inline void forward(AsyncResult& result);

protected:
    AsyncResult mForwardedResult;
};

/**
 * Implementation of IAsyncTask that uses results from previous tasks to run this task. Holds the result, since this is not a terminal task
 */
class ASYNC_API AsyncForwardTask : public IForwardingTask {
public:
    /**
     * Create an asynchronous task that takes in a previous AsyncResult and returns an AsyncResult via a packaged task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    AsyncForwardTask(std::function<void(AsyncResult&)> generateResult);

protected:
    virtual void performSpecific();

private:
    std::function<void(AsyncResult)> mGenerateResultFunc;
    
};

/**
 * Implementation of IAsyncTask that waits for a previous result from tasks
 */
class ASYNC_API AsyncTerminalTask : public IForwardingTask {
public:
    /**
     * Create an asynchronous task that takes in a previous AsyncResult and returns an AsyncResult via a packaged task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    AsyncTerminalTask(std::function<AsyncResult(AsyncResult&)> generateResult);

    /**
     * Get the future that will hold the result of this terminal task
     */
    inline AsyncFuture future();

protected:
    virtual void performSpecific();

private:
    std::function<AsyncResult(AsyncResult)> mGenerateResultFunc;
    std::promise<AsyncResult> mPromise;
};

//inline implementations
//------------------------------------------------------------------------------
void IForwardingTask::forward(AsyncResult& result)
{
    mForwardedResult = std::move(result);
}

//------------------------------------------------------------------------------
AsyncFuture AsyncTerminalTask::future()
{
    return mPromise.get_future();
}

}
}