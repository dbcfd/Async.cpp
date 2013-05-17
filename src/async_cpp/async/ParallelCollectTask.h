#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelTerminalTask.h"

#include <functional>
#include <vector>

namespace async_cpp {
namespace async {

/**
 * Task which collects a set of futures from parallel tasks.
 */
template<class TDATA, class TRESULT = TDATA>
class ParallelCollectTask : public IParallelTask {
public:
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding, 
        std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> generateResult);
    ParallelCollectTask(ParallelCollectTask&& other);
    virtual ~ParallelCollectTask();

    size_t notifyTaskCompletion(std::future<AsyncResult<TDATA>>&& futureResult);
    std::future<AsyncResult<TRESULT>> getFuture();
protected:
    virtual void performSpecific();
    virtual void notifyFailureToPerform();

private:
    std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> mGenerateResultFunc;
    std::mutex mTasksMutex;
    size_t mTasksOutstanding;
    std::vector<std::future<AsyncResult<TDATA>>> mTaskFutures;
    std::vector<AsyncResult<TDATA>> mTaskResults;
    std::shared_ptr<ParallelTerminalTask<TRESULT>> mTerminalTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelCollectTask<TDATA, TRESULT>::ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
                                           const size_t tasksOutstanding, 
                                           std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> generateResult)
                                           : IParallelTask(mgr), 
                                           mTasksOutstanding(tasksOutstanding), 
                                           mGenerateResultFunc(generateResult), 
                                           mTerminalTask(std::make_shared<ParallelTerminalTask<TRESULT>>(mgr))
{
    
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelCollectTask<TDATA, TRESULT>::ParallelCollectTask(ParallelCollectTask&& other)
                                           : IParallelTask(other.mManager),
                                           mGenerateResultFunc(std::move(other.mGenerateResultFunc)), 
                                           mTerminalTask(std::move(other.mTerminalTask)), 
                                           mTaskResults(std::move(other.mTaskResults)), 
                                           mTaskFutures(std::move(other.mTaskFutures))
{
    
    
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelCollectTask<TDATA, TRESULT>::~ParallelCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelCollectTask<TDATA, TRESULT>::performSpecific()
{
    if(mTaskFutures.empty())
    {
        try {
            mTerminalTask->forwardResult(mGenerateResultFunc(mTaskResults));
        }
        catch(std::exception& ex)
        {
            mTerminalTask->forwardResult(AsyncResult<TRESULT>(ex.what()).asFulfilledFuture());
        }
        if(auto mgr = mManager.lock())
        {
            mgr->run(mTerminalTask);
        }
        else
        {
            mTerminalTask->failToPerform();
        }
    }
    else
    {
        std::vector<std::future<AsyncResult<TDATA>>> futuresOutstanding;
        for(auto& future : mTaskFutures)
        {
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
            if(future._Is_ready())
#else
            if(std::future_status::ready == future.wait_for(std::chrono::milliseconds(0)))
#endif
            {
                mTaskResults.emplace_back(future.get());
            }
            else
            {
                futuresOutstanding.emplace_back(std::move(future));
            }
        }
        mTaskFutures.swap(futuresOutstanding);
        if(auto mgr = mManager.lock())
        {
            mgr->run(std::make_shared<ParallelCollectTask<TDATA, TRESULT>>(std::move(*this)));
        }
        else
        {
            notifyFailureToPerform();
        }
    }   
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelCollectTask<TDATA, TRESULT>::notifyFailureToPerform()
{
    mTerminalTask->failToPerform();
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
size_t ParallelCollectTask<TDATA, TRESULT>::notifyTaskCompletion(std::future<AsyncResult<TDATA>>&& futureResult)
{
    std::unique_lock<std::mutex> lock(mTasksMutex);
    mTaskFutures.emplace_back(std::move(futureResult));
    --mTasksOutstanding;
    return mTasksOutstanding;
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> ParallelCollectTask<TDATA, TRESULT>::getFuture()
{
    return mTerminalTask->getFuture();
}

}
}