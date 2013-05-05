#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelCollectTask.h"

#include <functional>
#include <vector>

namespace async_cpp {
namespace async {

/**
 * Parallel running task
 */
template<class TDATA, class TRESULT>
class ParallelTask : public IParallelTask {
public:
    ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TDATA, TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific();

private:
    std::function<std::future<AsyncResult<TDATA>>(void)> mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask<TDATA, TRESULT>> mCollectTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelTask<TDATA, TRESULT>::ParallelTask(std::shared_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TDATA, TRESULT>> collectTask)
    : IParallelTask(mgr), mGenerateResultFunc(std::move(generateResult)), mCollectTask(collectTask)
{
    assert(collectTask);
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelTask<TDATA, TRESULT>::~ParallelTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelTask<TDATA, TRESULT>::performSpecific()
{
    auto tasksRemaining = mCollectTask->notifyTaskCompletion(mGenerateResultFunc());
    if(0 == tasksRemaining)
    {
        mManager->run(mCollectTask);
    }
}

}
}