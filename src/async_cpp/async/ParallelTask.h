#pragma once
#include "async_cpp/async/Platform.h"
#include "async_cpp/async/Async.h"
#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/ParallelCollectTask.h"

#include "async_cpp/tasks/IManager.h"
#include "async_cpp/tasks/Task.h"

#include <functional>
#include <vector>

namespace async_cpp {
namespace async {

/**
 * Parallel running task
 */
template<class TDATA>
class ParallelTask : public tasks::Task {
public:
    ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TDATA>> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific();

private:
    std::shared_ptr<tasks::IManager> mManager;
    std::function<std::future<AsyncResult<TDATA>>(void)> mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask<TDATA>> mCollectTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ParallelTask<TDATA>::ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TDATA>> collectTask)
    : IParallelTask(mgr), mGenerateResultFunc(std::move(generateResult)), mCollectTask(collectTask)
{
    assert(mgr);
    assert(collectTask);
}

//------------------------------------------------------------------------------
template<class TDATA>
ParallelTask<TDATA>::~ParallelTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelTask<TDATA>::performSpecific()
{
    auto tasksRemaining = mCollectTask->notifyTaskCompletion(mGenerateResultFunc());
    if(0 == tasksRemaining)
    {
        mManager->run(mCollectTask);
    }
}

}
}