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
    ParallelCollectTask(std::shared_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding, 
        std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> generateResult);
    virtual ~ParallelCollectTask();

    size_t notifyTaskCompletion(std::future<AsyncResult<TDATA>>&& futureResult);
    std::future<AsyncResult<TRESULT>> getFuture();
protected:
    virtual void performSpecific();

private:
    std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> mGenerateResultFunc;
    std::mutex mTasksMutex;
    size_t mTasksOutstanding;
    std::vector<std::future<AsyncResult<TDATA>>> mTaskFutures;
    std::vector<AsyncResult<TDATA>> mTaskResults;
    std::shared_ptr<ParallelTerminalTask<TDATA>> mTerminalTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelCollectTask<TDATA, TRESULT>::ParallelCollectTask(std::shared_ptr<tasks::IManager> mgr,
                                           const size_t tasksOutstanding, 
                                           std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> generateResult)
                                           : IParallelTask(mgr), mTasksOutstanding(tasksOutstanding), mGenerateResultFunc(generateResult), 
                                           mTerminalTask(std::make_shared<ParallelTerminalTask<TRESULT>>(mgr))
{
    assert(mgr);
    mTaskResults.reserve(tasksOutstanding);
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
        std::future<AsyncResult<TRESULT>> futureToForward;
        try {
            futureToForward = mGenerateResultFunc(mTaskResults);
        }
        catch(std::exception& ex)
        {
            futureToForward = AsyncResult<TDATA>(ex.what()).asFulfilledFuture();
        }
        mTerminalTask->forwardResult(std::move(futureToForward));
        mManager->run(mTerminalTask);
    }
    else
    {
        std::vector<std::future<AsyncResult<TDATA>>> futuresOutstanding;
        for(auto& future : mTaskFutures)
        {
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
            if(mGeneratedFuture._Is_ready())
#else
            if(std::future_status::ready == mGeneratedFuture.wait_for(std::chrono::milliseconds(0)))
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
        reset();
        mManager->run(shared_from_this());
    }   
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
size_t ParallelCollectTask<TDATA, TRESULT>::notifyTaskCompletion(std::future<AsyncResult<TDATA>>&& futureResult)
{
    std::unique_lock<std::mutex> lock(mTasksMutex);
    mTaskResults.emplace_back(std::move(futureResult));
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