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
//------------------------------------------------------------------------------
template<class TRESULT>
class Parallel {
public:
    typedef typename detail::ParallelTask<TRESULT>::callback_t callback_t;
    typedef std::function<void(typename callback_t)> operation_t; 
    typedef typename detail::ParallelCollectTask<TRESULT>::then_t then_t;
    typedef typename detail::ParallelCollectTask<TRESULT>::result_set_t result_set_t;
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
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return AsyncResult that holds a future completion status, either successful or exception
     */
    AsyncResult then(typename then_t thenFunc);

    /**
     * Cancel all running tasks.
     */
    void cancel();

private:
    std::vector<typename operation_t> mOps;
    std::vector<std::shared_ptr<detail::IParallelTask<TRESULT>>> mTasks;
    tasks::ManagerPtr mManager;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
Parallel<TRESULT>::Parallel(tasks::ManagerPtr manager, 
                                   const std::vector<typename operation_t>& tasks)
    : mManager(manager), mOps(tasks)
{
    if(!mManager) { throw(std::invalid_argument("Parallel: Manager cannot be null")); }
    if(mOps.empty()) { throw(std::invalid_argument("Parallel: Must have at least one task")); }
}

//------------------------------------------------------------------------------
template<class TRESULT>
Parallel<TRESULT>::Parallel(tasks::ManagerPtr manager, 
                                   typename operation_t tasks[], const size_t nbTasks)
    : mManager(manager)
{
    if(!mManager) { throw(std::invalid_argument("Parallel: Manager cannot be null")); }
    if(!tasks) { throw(std::invalid_argument("Parallel: Must have at least one task")); }

    mOps.assign(tasks, tasks+nbTasks);
}

//------------------------------------------------------------------------------
template<class TRESULT>
AsyncResult Parallel<TRESULT>::then(typename then_t thenFunc)
{
    auto terminalTask(std::make_shared<detail::ParallelCollectTask<TRESULT>>(mManager, mOps.size(), thenFunc));
    mTasks.reserve(mOps.size() + 1);
    mTasks.emplace_back(terminalTask);

    auto result = terminalTask->result();

    for(size_t i = 0; i < mOps.size(); ++i)
    {
        auto callback = [terminalTask, i](typename detail::IParallelTask<TRESULT>::VariantType result)->void
        {
            terminalTask->notifyCompletion(i, std::move(result));
        };
        auto op = std::bind(mOps[i], callback);
        mTasks.emplace_back(std::make_shared<detail::ParallelTask<TRESULT>>(mManager, op, terminalTask));
        mManager->run(mTasks.back());
    }

    return result; 
}

//------------------------------------------------------------------------------
template<class TRESULT>
void Parallel<TRESULT>::cancel()
{
    for(auto task : mTasks)
    {
        task->cancel();
    }
}

}
}