#include "async_cpp/async/ParallelTask.h"
#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/tasks/IManager.h"
#include "async_cpp/tasks/BasicTask.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelTask::ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<AsyncFuture(void)> generateResult,
        std::shared_ptr<ParallelCollectTask> collectTask)
    : mManager(mgr), mGenerateResultFunc(std::move(generateResult)), mCollectTask(collectTask)
{
    assert(mgr);
    assert(collectTask);
}

//------------------------------------------------------------------------------
ParallelTask::~ParallelTask()
{

}

//------------------------------------------------------------------------------
void ParallelTask::performSpecific()
{
    auto tasksRemaining = mCollectTask->notifyTaskCompletion(mGenerateResultFunc());
    if(0 == tasksRemaining)
    {
        mManager->run(mCollectTask);
    }
}

//------------------------------------------------------------------------------
ParallelCollectTask::ParallelCollectTask(std::shared_ptr<tasks::IManager> mgr,
                                           const size_t tasksOutstanding, 
                                           std::function<AsyncFuture(std::vector<AsyncResult>&)> generateResult)
                                           : mManager(mgr), mTasksOutstanding(tasksOutstanding), mGenerateResultFunc(generateResult), 
                                           mTerminalTask(std::make_shared<ParallelTerminalTask>())
{
    assert(mgr);
    mTaskResults.reserve(tasksOutstanding);
}

//------------------------------------------------------------------------------
ParallelCollectTask::~ParallelCollectTask()
{

}

//------------------------------------------------------------------------------
void ParallelCollectTask::performSpecific()
{
    std::vector<AsyncResult> results;
    for(auto& future : mTaskResults)
    {
        try 
        {
            results.emplace_back(future.get());
        }
        catch(std::future_error& ex)
        {
            results.emplace_back(AsyncResult(ex.what()));
        }
    }
    AsyncFuture futureToForward;
    try {
        futureToForward = mGenerateResultFunc(results);
    }
    catch(std::exception& ex)
    {
        futureToForward = AsyncResult(ex.what()).asFulfilledFuture();
    }
    mTerminalTask->forwardResult(std::move(futureToForward));
    mManager->run(mTerminalTask);
}

//------------------------------------------------------------------------------
ParallelTerminalTask::ParallelTerminalTask()
{

}

//------------------------------------------------------------------------------
ParallelTerminalTask::~ParallelTerminalTask()
{

}

//------------------------------------------------------------------------------
void ParallelTerminalTask::performSpecific()
{
    AsyncResult res;
    try
    {
        res = mGeneratedFuture.get();
    }
    catch(std::future_error& ex)
    {
        res = AsyncResult(ex.what());
    }
    mPromise.set_value(res);
}

//------------------------------------------------------------------------------
AsyncFuture ParallelTerminalTask::getFuture()
{
    AsyncFuture ret;
    try 
    {
        ret = mPromise.get_future();
    }
    catch(std::future_error& error)
    {
        ret = AsyncResult(error.what()).asFulfilledFuture();
    }
    return ret;
}

}
}
