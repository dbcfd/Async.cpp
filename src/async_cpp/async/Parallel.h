#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/detail/ParallelTask.h"

namespace async_cpp {
namespace async {

/**
 * Run a set of tasks in parallel using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
template<class TDATA, class TRESULT = TDATA>
class Parallel {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    Parallel(std::shared_ptr<tasks::IManager> manager, const std::vector<std::function<std::future<AsyncResult<TDATA>>(void)>>& tasks);
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Array of tasks that will be run
     * @param nbTasks Number of tasks in array
     */
    Parallel(std::shared_ptr<tasks::IManager> manager, std::function<std::future<AsyncResult<TDATA>>(void)> tasks[], const size_t nbTasks);

    /**
     * Run the set of tasks in parallel, calling a task when the parallel tasks have completed.
     * @param onFinishTask Task to run when parallel tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> onFinishTask) const;

private:
    std::vector<std::function<std::future<AsyncResult<TDATA>>(void)>> mOps;
    std::shared_ptr<tasks::IManager> mManager;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Parallel<TDATA, TRESULT>::Parallel(std::shared_ptr<tasks::IManager> manager, const std::vector<std::function<std::future<AsyncResult<TDATA>>(void)>>& tasks)
    : mManager(manager), mOps(tasks)
{
    assert(manager);
    assert(!tasks.empty());
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Parallel<TDATA, TRESULT>::Parallel(std::shared_ptr<tasks::IManager> manager, std::function<std::future<AsyncResult<TDATA>>(void)> tasks[], const size_t nbTasks)
    : mManager(manager)
{
    assert(manager);
    assert(tasks);

    mOps.assign(tasks, tasks+nbTasks);

    assert(!mOps.empty());
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> Parallel<TDATA, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> onFinishOp) const
{
    auto terminalTask(std::make_shared<detail::ParallelCollectTask<TDATA, TRESULT>>(mManager, mOps.size(), onFinishOp));

    auto future = terminalTask->getFuture();

    for(auto op : mOps)
    {
        mManager->run(std::make_shared<detail::ParallelTask<TDATA, TRESULT>>(mManager, op, terminalTask));
    }

    return future; 
}

}
}