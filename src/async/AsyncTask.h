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

//have to pass results as shared_ptr since std::bind is not move or forward
typedef std::shared_ptr<AsyncResult> PtrAsyncResult;
typedef std::future<PtrAsyncResult> AsyncFuture;

//------------------------------------------------------------------------------
class IAsyncTask : public workers::Task {
public:
    IAsyncTask();
    virtual ~IAsyncTask();

    virtual AsyncFuture getFuture() = 0;
};

//------------------------------------------------------------------------------
class AsyncTask : public IAsyncTask {
public:
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