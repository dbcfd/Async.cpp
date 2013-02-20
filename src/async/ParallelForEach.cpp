#include "async/ParallelForEach.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelForEach::ParallelForEach(std::shared_ptr<workers::IManager> manager, 
        std::function<PtrAsyncResult(std::shared_ptr<void>)> op, 
        const std::vector<std::shared_ptr<void>>& data)
    : mManager(manager), mData(data), mOp(op)
{
    assert(nullptr != manager);
    
}

//------------------------------------------------------------------------------
AsyncFuture ParallelForEach::execute(std::function<PtrAsyncResult(const std::vector<PtrAsyncResult>&)> onFinishTask)
{
    std::shared_ptr<std::vector<AsyncFuture>> taskFutures(new std::vector<AsyncFuture>());

    std::shared_ptr<AsyncTask> finishTask(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(
        [taskFutures, onFinishTask]()->PtrAsyncResult {
            std::vector<PtrAsyncResult> taskResults;
            taskResults.reserve(taskFutures->size());
            for(auto& taskFuture : (*taskFutures))
            {
                taskResults.push_back(taskFuture.get());
            }
            PtrAsyncResult result;
            try 
            {
                result = onFinishTask(taskResults);
            }
            catch(...)
            {
                result = PtrAsyncResult(new AsyncResult("Error in function invoked on finish"));
            }
            return result;
        }
    ) ) );

    taskFutures->reserve(mData.size());
    std::shared_ptr<std::atomic<size_t>> opsRemaining(new std::atomic<size_t>(mData.size()));

    for(auto dataForOp : mData)
    {
        auto task = std::shared_ptr<AsyncTask>(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(std::bind(
            [dataForOp, opsRemaining, finishTask](std::shared_ptr<workers::IManager> mgr, std::function<PtrAsyncResult(std::shared_ptr<void>)> opCopy)->PtrAsyncResult 
            {
                PtrAsyncResult res = opCopy(dataForOp);
                if(1 == opsRemaining->fetch_sub(1))
                {
                    //last op
                    mgr->run(finishTask);
                }
                return res;
            }, mManager, mOp ) ) ) );
        taskFutures->push_back(task->getFuture());
        mManager->run(task);
    }

    return finishTask->getFuture();  
}

//------------------------------------------------------------------------------
AsyncFuture ParallelForEach::execute()
{
    return execute([](const std::vector<PtrAsyncResult>& taskResults)->PtrAsyncResult {
        PtrAsyncResult result(new AsyncResult());
        for(PtrAsyncResult taskResult : taskResults)
        {
            if(taskResult->wasError() && !result->wasError())
            {
                result = std::move(taskResult);
                break;
            }
        }
        return result;
    } );
}

}
}
