#pragma once
#include "async/Platform.h"

#include "workers/Task.h"

#include <future>
#include <memory>
#include <vector>

namespace quicktcp {

namespace workers {
class Manager;
}

namespace async {

class AsyncResult;

//have to pass results as shared_ptr since std::bind and lambda is not move or forward aware
typedef std::shared_ptr<AsyncResult> PtrAsyncResult;
typedef std::future<PtrAsyncResult> AsyncFuture;

/**
 * Asynchronous task interface. Provides task interface and methodology to retrieve future
 */
class IAsyncTask : public workers::Task {
public:
    IAsyncTask();
    virtual ~IAsyncTask();

    /**
     * Return the future that this task creates.
     * @return Future created
     */
    virtual AsyncFuture getFuture() = 0;
};

/**
 * Implementation of IAsyncTask that does receive forwarded results from other tasks.
 */
class AsyncTask : public IAsyncTask {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    AsyncTask(std::packaged_task<PtrAsyncResult(void)> generateResult);

    inline virtual AsyncFuture getFuture();

protected:
    virtual void performSpecific();

private:
    std::packaged_task<PtrAsyncResult(void)> mGenerateResultFunc;
};

//------------------------------------------------------------------------------
class AsyncForwardTask : public IAsyncTask {
public:
    /**
     * Create an asynchronous task that takes in a previous AsyncResult and returns an AsyncResult via a packaged task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    AsyncForwardTask(AsyncFuture forwardedFuture, 
        std::packaged_task<PtrAsyncResult(PtrAsyncResult)> generateResult);

    inline virtual AsyncFuture getFuture();

protected:
    virtual void performSpecific();

private:
    std::future<PtrAsyncResult> mForwardedFuture;
    std::packaged_task<PtrAsyncResult(PtrAsyncResult)> mGenerateResultFunc;
};

//inline implementations
//------------------------------------------------------------------------------
AsyncFuture AsyncTask::getFuture()
{
    return mGenerateResultFunc.get_future();
}

//------------------------------------------------------------------------------
AsyncFuture AsyncForwardTask::getFuture()
{
    return mGenerateResultFunc.get_future();
}

}
}