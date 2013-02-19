#include "async/ParallelForEach.h"
#include "async/AsyncResult.h"

#include "workers/Manager.h"

namespace quicktcp {
namespace async {

//------------------------------------------------------------------------------
ParallelForEach::ParallelForEach(std::shared_ptr<workers::Manager> manager, 
        std::function<PtrAsyncResult(std::shared_ptr<void>)> op, 
        const std::vector<std::shared_ptr<void>>& data)
    : mManager(manager), mData(data)
{
    mTasks.reserve(data.size());
    for(auto dataForOp : data)
    {
        mTasks.emplace_back(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(
            [dataForOp, op]()->PtrAsyncResult {
                return op(dataForOp);
            }
        ) ) );
    }
}

//------------------------------------------------------------------------------
AsyncFuture ParallelForEach::execute(std::function<PtrAsyncResult(const std::vector<PtrAsyncResult>&)> onFinishTask)
{
    std::shared_ptr<std::vector<AsyncFuture>> taskFutures(new std::vector<AsyncFuture>());
    taskFutures->reserve(mTasks.size());

    for(auto task : mTasks)
    {
        taskFutures->push_back(task->getFuture());
        mManager->run(task);
    }

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

    mManager->run(finishTask);

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
