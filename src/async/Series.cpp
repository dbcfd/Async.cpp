#include "async/Series.h"
#include "async/AsyncResult.h"

#include "workers/Manager.h"

namespace quicktcp {
namespace async {

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::Manager> manager, const std::vector<std::function<PtrAsyncResult(PtrAsyncResult)>>& tasks)
    : mManager(manager)
{
    if(!tasks.empty())
    {
        mTasks.reserve(tasks.size());

        addTask(tasks.front());

        for(auto iter = tasks.begin() + 1; iter != tasks.end(); ++iter)
        {
            addTask(mTasks.back()->getFuture(), (*iter));
        }
    }
}

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::Manager> manager, std::function<PtrAsyncResult(PtrAsyncResult)>* tasks, const size_t nbTasks)
    : mManager(manager)
{
    if(0 != nbTasks)
    {
        mTasks.reserve(nbTasks);

        addTask(tasks[0]);

        for(size_t idx = 1; idx < nbTasks; ++idx)
        {
            addTask(mTasks.back()->getFuture(), tasks[idx]);
        }
    }
}

//------------------------------------------------------------------------------
void Series::addTask(std::function<PtrAsyncResult(PtrAsyncResult)> func)
{
    std::packaged_task<PtrAsyncResult(void)> packagedTask(
        [func]() -> PtrAsyncResult {
            return func(PtrAsyncResult(new AsyncResult()));
    } );
    mTasks.emplace_back(new AsyncTask(std::move(packagedTask)));
}

//------------------------------------------------------------------------------
void Series::addTask(std::future<PtrAsyncResult> forwardedFuture, std::function<PtrAsyncResult(PtrAsyncResult)> func)
{
    std::packaged_task<PtrAsyncResult(PtrAsyncResult)> packagedTask(
        [func](PtrAsyncResult result) -> PtrAsyncResult {
            return func(result);
    } );
    mTasks.emplace_back(new AsyncForwardTask(std::move(forwardedFuture), std::move(packagedTask)));
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask)
{
    std::shared_ptr<IAsyncTask> task;

    if(mTasks.empty())
    {
        std::packaged_task<PtrAsyncResult(void)> finishTask( [onFinishTask]() -> PtrAsyncResult {
            return onFinishTask(PtrAsyncResult(new AsyncResult()));
        } );
        task = std::shared_ptr<IAsyncTask>(new AsyncTask(std::move(finishTask)));
    }
    else
    {
        for(auto task : mTasks)
        {
            mManager->run(task);
        }
        std::packaged_task<PtrAsyncResult(PtrAsyncResult)> finishTask( [this, onFinishTask](PtrAsyncResult res) -> PtrAsyncResult {
            PtrAsyncResult result;
            try {
                result = onFinishTask(std::move(res));
            }
            catch(...)
            {
                result = PtrAsyncResult(new AsyncResult("Unknown error"));
            }
            return result;
        } );
        task = std::shared_ptr<IAsyncTask>(new AsyncForwardTask(mTasks.back()->getFuture(), std::move(finishTask)));
    }

    mManager->run(task);

    return task->getFuture();  
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute()
{
    return execute([](PtrAsyncResult in)->PtrAsyncResult {
        return std::move(in);
    } );
}

}
}
