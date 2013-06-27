#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/detail/SeriesCollectTask.h"
#include "async_cpp/async/detail/SeriesTask.h"

#include <functional>

namespace async_cpp {
namespace async {

/**
 * Run a set of tasks in series using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
template<class TDATA, class TRESULT = TDATA>
class Series {
public:
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Vector of tasks that will be run
     */
    Series(std::shared_ptr<tasks::IManager> manager, 
        const std::vector<std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)>>& ops);
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Array of operations that will be run
     * @param nbOps Number of operations in array
     */
    Series(std::shared_ptr<tasks::IManager> manager, 
        std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)>* ops, const size_t nbOps);

    /**
     * Run the set of tasks in series, calling a task when the series tasks have completed.
     * @param onFinishTask Task to run when series tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> onFinishTask) const;

private:
    std::vector<std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)>> mOperations;
    std::shared_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Series<TDATA, TRESULT>::Series(std::shared_ptr<tasks::IManager> manager, 
               const std::vector<std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)>>& ops)
    : mManager(manager), mOperations(ops)
{
    assert(manager);
    assert(!ops.empty());
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Series<TDATA, TRESULT>::Series(std::shared_ptr<tasks::IManager> manager, 
               std::function<std::future<AsyncResult<TDATA>>(const AsyncResult<TDATA>&)>* ops, const size_t nbOps)
    : mManager(manager)
{
    assert(manager);
    assert(ops);

    mOperations.assign(ops, ops+nbOps);
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> Series<TDATA, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const AsyncResult<TDATA>&)> onFinishOp) const
{
    auto finishTask(std::make_shared<SeriesCollectTask<TDATA, TRESULT>>(mManager, onFinishOp));

    auto result = finishTask->getFuture();

    std::shared_ptr<ISeriesTask<TDATA>> nextTask = finishTask;
    for(auto iter = mOperations.rbegin(); iter != mOperations.rend(); ++iter)
    {
        nextTask = std::make_shared<SeriesTask<TDATA>>(mManager, (*iter), nextTask);
    }

    nextTask->forwardFuture(AsyncResult<TDATA>().asFulfilledFuture());
    mManager->run(nextTask);

    return result;  
}

}
}