#include "async/Parallel.h"
#include "async/AsyncResult.h"

#include "workers/Manager.h"

namespace quicktcp {
namespace async {

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::Manager> manager, const std::vector<std::function<PtrAsyncResult(void)>>& tasks)
    : mManager(manager)
{
    mTasks.reserve(tasks.size());
    for(auto func : tasks)
    {
        mTasks.emplace_back(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(
            [func]()->PtrAsyncResult {
                return func();
            } ) ) );
    }
}

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::Manager> manager, std::function<PtrAsyncResult(void)>* tasks, const size_t nbTasks)
    : mManager(manager)
{
    mTasks.reserve(nbTasks);
    for(size_t idx = 0; idx < nbTasks; ++idx)
    {
        mTasks.emplace_back(new AsyncTask(std::packaged_task<PtrAsyncResult(void)>(
            [tasks, idx]()->PtrAsyncResult {
                return tasks[idx]();
            } ) ) );
    }
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask)
{
    std::shared_ptr<std::vector<AsyncFuture>> taskFutures(new std::vector<AsyncFuture>());
    taskFutures->reserve(mTasks.size());

    for(auto task : mTasks)
    {
        taskFutures->push_back(task->getFuture());
        mManager->run(task);
    }

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


    mManager->run(finishTask);

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
