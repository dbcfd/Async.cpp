#include "async/Series.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<PtrAsyncResult(PtrAsyncResult)>>& tasks)
    : mManager(manager), mTasks(new std::vector<std::shared_ptr<IAsyncTask>>())
{
    assert(nullptr != manager);

    //add padding for the finish task
    mTasks->resize(tasks.size() + 1);

    if(!tasks.empty())
    {
        mTasks->at(0).swap(addTask(tasks[0], 1));

        for(size_t idx = 1; idx < tasks.size(); ++idx)
        {
            mTasks->at(idx).swap(addTask(mTasks->at(idx-1)->getFuture(), tasks[idx], idx));
        }
    }
}

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, std::function<PtrAsyncResult(PtrAsyncResult)>* tasks, const size_t nbTasks)
    : mManager(manager), mTasks(new std::vector<std::shared_ptr<IAsyncTask>>())
{
    assert(nullptr != manager);

    //add padding for the finish task
    mTasks->resize(nbTasks + 1);

    if(0 != nbTasks)
    {
        mTasks->at(0).swap(addTask(tasks[0], 1));

        for(size_t idx = 1; idx < nbTasks; ++idx)
        {
            mTasks->at(idx).swap(addTask(mTasks->at(idx-1)->getFuture(), tasks[idx], idx+1));
        }
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<IAsyncTask> Series::addTask(std::function<PtrAsyncResult(PtrAsyncResult)> func, const size_t nextIndex)
{
    std::packaged_task<PtrAsyncResult(void)> packagedTask(std::bind(
        [func, nextIndex](std::shared_ptr<workers::IManager> mgr, std::shared_ptr<std::vector<std::shared_ptr<IAsyncTask>>> tasks) -> PtrAsyncResult {
            PtrAsyncResult res = func(PtrAsyncResult(new AsyncResult()));
            mgr->run(tasks->at(nextIndex));
            return res;
    }, mManager, mTasks) );
    return std::shared_ptr<IAsyncTask>(new AsyncTask(std::move(packagedTask)));
}

//------------------------------------------------------------------------------
std::shared_ptr<IAsyncTask> Series::addTask(std::future<PtrAsyncResult> forwardedFuture, std::function<PtrAsyncResult(PtrAsyncResult)> func, const size_t nextIndex)
{
    std::packaged_task<PtrAsyncResult(PtrAsyncResult)> packagedTask(std::bind(
        [func, nextIndex](std::shared_ptr<workers::IManager> mgr, std::shared_ptr<std::vector<std::shared_ptr<IAsyncTask>>> tasks, PtrAsyncResult result) -> PtrAsyncResult {
            PtrAsyncResult res = func(result);
            mgr->run(tasks->at(nextIndex));
            return res;
    }, mManager, mTasks, std::placeholders::_1) );
    return std::shared_ptr<IAsyncTask>(new AsyncForwardTask(std::move(forwardedFuture), std::move(packagedTask)));
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute(std::function<PtrAsyncResult(PtrAsyncResult)> onFinishTask)
{
    std::shared_ptr<IAsyncTask> finishTask;

    if(mTasks->size() == 1)
    {
        std::packaged_task<PtrAsyncResult(void)> finishOp( [onFinishTask]() -> PtrAsyncResult {
            return onFinishTask(PtrAsyncResult(new AsyncResult()));
        } );
        finishTask = std::shared_ptr<IAsyncTask>(new AsyncTask(std::move(finishOp)));
    }
    else
    {
        std::packaged_task<PtrAsyncResult(PtrAsyncResult)> finishOp( [this, onFinishTask](PtrAsyncResult res) -> PtrAsyncResult {
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
        finishTask = std::shared_ptr<IAsyncTask>(new AsyncForwardTask(mTasks->at(mTasks->size() - 2)->getFuture(), std::move(finishOp)));
    }

    mTasks->at(mTasks->size() - 1).swap(finishTask);

    mManager->run(mTasks->front());

    return mTasks->back()->getFuture();  
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
