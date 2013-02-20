#include "async/Parallel.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<PtrAsyncResult(void)>>& tasks)
    : mManager(manager), mOps(tasks)
{
    assert(nullptr != manager);
}

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, std::function<PtrAsyncResult(void)> tasks[], const size_t nbTasks)
    : mManager(manager)
{
    assert(nullptr != manager);

    mOps.assign(tasks, tasks+nbTasks);
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask)
{
    std::shared_ptr<std::vector<AsyncFuture>> taskFutures(new std::vector<AsyncFuture>());

    std::shared_ptr<AsyncTask> finishTask(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(
        [onFinishTask, taskFutures]()-> PtrAsyncResult {
            PtrAsyncResult result(new AsyncResult());
            for(AsyncFuture& taskFuture : (*taskFutures))
            {
                PtrAsyncResult taskResult(taskFuture.get());
                if(taskResult->wasError() && !result->wasError())
                {
                    result = std::move(taskResult);
                }
            }
            try 
            {
                result = onFinishTask(std::move(result));
            }
            catch(...)
            {
                result = PtrAsyncResult(new AsyncResult("Error in function invoked on finish"));
            }
            return result;
        }
    ) ) );

    taskFutures->reserve(mOps.size());
    std::shared_ptr<std::atomic<size_t>> opsRemaining(new std::atomic<size_t>(mOps.size()));

    for(auto op : mOps)
    {
        auto task = std::shared_ptr<IAsyncTask>(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(std::bind(
            [op, opsRemaining, finishTask](std::shared_ptr<workers::IManager> mgr)->PtrAsyncResult {
                auto res = op();
                if(1 == opsRemaining->fetch_sub(1))
                {
                    mgr->run(finishTask);
                }
                return res;
            }, mManager) ) ) );
        taskFutures->push_back(task->getFuture());
        mManager->run(task);
    }

    return finishTask->getFuture();  
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute()
{
    return execute([](PtrAsyncResult in)->PtrAsyncResult {
        return in;
    } );
}

}
}
