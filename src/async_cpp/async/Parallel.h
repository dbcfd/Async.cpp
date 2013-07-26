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
template<class TDATA, class TRESULT=TDATA>
class Parallel {
public:
    typedef typename typename detail::ParallelTask<TDATA, TRESULT>::operation_t operation_t; 
    typedef typename typename detail::ParallelTask<TDATA, TRESULT>::callback_t callback_t;
    typedef typename typename detail::ParallelCollectTask<TDATA>::then_t then_t;
    typedef typename typename detail::ParallelCollectTask<TDATA>::callback_t complete_t;
    typedef typename typename detail::ParallelCollectTask<TRESULT>::result_set_t result_set_t;
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    Parallel(tasks::ManagerPtr manager, const std::vector<operation_t>& tasks);
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Array of tasks that will be run
     * @param nbTasks Number of tasks in array
     */
    Parallel(tasks::ManagerPtr manager, typename operation_t tasks[], const size_t nbTasks);

    /**
     * Run the set of tasks in parallel, calling a task when the parallel tasks have completed.
     * @param thenFunc Function to invoke when all tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult> then(typename then_t thenFunc);

    /**
     * Cancel all running tasks.
     */
    void cancel();

private:
    std::vector<typename operation_t> mOps;
    std::vector<std::shared_ptr<detail::IParallelTask>> mTasks;
    tasks::ManagerPtr mManager;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Parallel<TDATA, TRESULT>::Parallel(tasks::ManagerPtr manager, 
                                   const std::vector<typename operation_t>& tasks)
    : mManager(manager), mOps(tasks)
{
    if(!mManager) { throw(std::invalid_argument("Parallel: Manager cannot be null")); }
    if(mOps.empty()) { throw(std::invalid_argument("Parallel: Must have at least one task")); }
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
Parallel<TDATA, TRESULT>::Parallel(tasks::ManagerPtr manager, 
                                   typename operation_t tasks[], const size_t nbTasks)
    : mManager(manager)
{
    if(!mManager) { throw(std::invalid_argument("Parallel: Manager cannot be null")); }
    if(!tasks) { throw(std::invalid_argument("Parallel: Must have at least one task")); }

    mOps.assign(tasks, tasks+nbTasks);
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult> Parallel<TDATA, TRESULT>::then(typename then_t thenFunc)
{
    auto terminalTask(std::make_shared<detail::ParallelCollectTask<TRESULT>>(mManager, mOps.size(), thenFunc));
    mTasks.reserve(mOps.size() + 1);
    mTasks.emplace_back(terminalTask);

    auto future = terminalTask->future();

    for(size_t i = 0; i < mOps.size(); ++i)
    {
        mTasks.emplace_back(std::make_shared<detail::ParallelTask<TDATA>>(mManager, mOps[i], i, terminalTask));
        mManager->run(mTasks.back());
    }

    return future; 
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void Parallel<TDATA, TRESULT>::cancel()
{
    for(auto task : mTasks)
    {
        task->cancel();
    }
}

}
}