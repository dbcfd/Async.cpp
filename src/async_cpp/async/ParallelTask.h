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
    ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<std::future<AsyncResult<TDATA>>(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TDATA, TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific();
    virtual void notifyFailureToPerform();

private:
    std::function<std::future<AsyncResult<TDATA>>(void)> mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask<TDATA, TRESULT>> mCollectTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelTask<TDATA, TRESULT>::ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
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
    std::future<AsyncResult<TDATA>> future;
    try 
    {
        future = mGenerateResultFunc();
    }
    catch(std::exception& ex)
    {
        future = AsyncResult<TDATA>(ex.what()).asFulfilledFuture();
    }
    catch(...)
    {
        future = AsyncResult<TDATA>("Parallel(For/Each): Unknown exception, please verify parallel tasks or use std::exception").asFulfilledFuture();
    }
    auto tasksRemaining = mCollectTask->notifyTaskCompletion(std::move(future));
    if(0 == tasksRemaining)
    {
        if(auto mgr = mManager.lock())
        {  
            mgr->run(mCollectTask);
        }
        else
        {
            mCollectTask->failToPerform();
        }
    }
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelTask<TDATA, TRESULT>::notifyFailureToPerform()
{
    auto tasksRemaining = mCollectTask->notifyTaskCompletion(AsyncResult<TDATA>("ParallelTask: Failed to perform").asFulfilledFuture());
    if(0 == tasksRemaining)
    {
        mCollectTask->failToPerform();
    }
}

}
}