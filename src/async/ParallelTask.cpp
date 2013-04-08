#include "async/ParallelTask.h"
#include "async/AsyncResult.h"
#include "workers/IManager.h"
#include "workers/BasicTask.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelTask::ParallelTask(std::shared_ptr<workers::IManager> mgr, 
        std::function<AsyncFuture(void)> generateResult,
        std::shared_ptr<ParallelCollectTask> collectTask)
    : mManager(mgr), mGenerateResultFunc(std::move(generateResult)), mCollectTask(collectTask)
{
    assert(mgr);
    assert(collectTask);
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
ParallelCollectTask::ParallelCollectTask(std::shared_ptr<workers::IManager> mgr,
                                           const size_t tasksOutstanding, 
                                           std::function<AsyncFuture(std::vector<AsyncResult>&)> generateResult)
                                           : mManager(mgr), mTasksOutstanding(tasksOutstanding), mGenerateResultFunc(generateResult), 
                                           mTerminalTask(new ParallelTerminalTask())
{
    assert(mgr);
    mTaskResults.reserve(tasksOutstanding);
}

//------------------------------------------------------------------------------
void ParallelCollectTask::performSpecific()
{
    std::vector<AsyncResult> results;
    for(auto& future : mTaskResults)
    {
        results.emplace_back(future.get());
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
void ParallelTerminalTask::performSpecific()
{
    mPromise.set_value(mGeneratedFuture.get());
}

}
}
