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

class ParallelCollectTask;

/**
 * Parallel running task
 */
class ASYNC_CPP_ASYNC_API ParallelTask : public tasks::Task {
public:
    ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<AsyncFuture(void)> generateResult,
        std::shared_ptr<ParallelCollectTask> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific();

private:
    std::shared_ptr<tasks::IManager> mManager;
    std::function<AsyncFuture(void)> mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask> mCollectTask;
};

class ParallelTerminalTask;

/**
 * Task which collects a set of futures from parallel tasks.
 */
class ASYNC_CPP_ASYNC_API ParallelCollectTask : public tasks::Task {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    ParallelCollectTask(std::shared_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding, 
        std::function<AsyncFuture(std::vector<AsyncResult>&)> generateResult);
    virtual ~ParallelCollectTask();

    inline size_t notifyTaskCompletion(AsyncFuture&& futureResult);
    inline AsyncFuture getFuture();
protected:
    virtual void performSpecific();

private:
    std::function<AsyncFuture(std::vector<AsyncResult>&)> mGenerateResultFunc;
    std::mutex mTasksMutex;
    size_t mTasksOutstanding;
    std::vector<AsyncFuture> mTaskResults;
    std::shared_ptr<tasks::IManager> mManager;
    std::shared_ptr<ParallelTerminalTask> mTerminalTask;
};

/**
 * Task which collects a set of futures from parallel tasks.
 */
class ASYNC_CPP_ASYNC_API ParallelTerminalTask : public tasks::Task {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    ParallelTerminalTask();
    virtual ~ParallelTerminalTask();

    AsyncFuture getFuture();

    inline void forwardResult(AsyncFuture&& futureResult);
protected:
    virtual void performSpecific();

private:
    std::promise<AsyncResult> mPromise;
    AsyncFuture mGeneratedFuture;
};

//inline implementations
//------------------------------------------------------------------------------
size_t ParallelCollectTask::notifyTaskCompletion(AsyncFuture&& futureResult)
{
    std::unique_lock<std::mutex> lock(mTasksMutex);
    mTaskResults.emplace_back(std::move(futureResult));
    --mTasksOutstanding;
    return mTasksOutstanding;
}

//------------------------------------------------------------------------------
AsyncFuture ParallelCollectTask::getFuture()
{
    return mTerminalTask->getFuture();
}

//------------------------------------------------------------------------------
void ParallelTerminalTask::forwardResult(AsyncFuture&& futureResult)
{
    mGeneratedFuture = std::move(futureResult);
}

}
}