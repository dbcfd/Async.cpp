#pragma once
#include "async_cpp/async/detail/ParallelTask.h"

namespace async_cpp {
namespace async {

/**
 * Perform an operation in parallel for a number of times, optionally calling a function to examine all results once parallel 
 * operations are complete. Each task will be passed an index as data.
 */
//------------------------------------------------------------------------------
template<class TDATA>
class ParallelFor {
public:
    typedef std::function<void(const size_t, typename detail::ParallelTask<TDATA>::callback_t)> operation_t;
    typedef typename detail::ParallelTask<TDATA>::callback_t callback_t;
    typedef typename detail::ParallelCollectTask<TDATA>::then_t then_t;
    typedef typename detail::ParallelCollectTask<TDATA>::result_set_t result_set_t;
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param op Operation to run in parallel for a number of times
     * @param nbTimes Number of times to run operation for
     */
    ParallelFor(tasks::ManagerPtr manager, 
        typename operation_t op, 
        const size_t nbTimes);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return AsyncResult that holds a future completion status, either successful or exception
     */
    AsyncResult then(typename then_t);

    /**
     * Cancel outstanding tasks
     */
    void cancel();

private:
    typename operation_t mOp;
    tasks::ManagerPtr mManager;
    std::vector<std::shared_ptr<detail::IParallelTask<TDATA>>> mTasks;
    size_t mNbTimes;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ParallelFor<TDATA>::ParallelFor(tasks::ManagerPtr manager, 
        typename operation_t op, 
        const size_t nbTimes)
    : mManager(manager), mOp(op), mNbTimes(nbTimes)
{
    if(!mManager) { throw(std::invalid_argument("ParallelFor: Manager cannot be null")); }
    if(0 == mNbTimes) { throw(std::invalid_argument("ParallelFor: At least one iteration required")); }
    
}

//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult ParallelFor<TDATA>::then(typename detail::ParallelCollectTask<TDATA>::then_t onFinishOp )
{
    auto terminalTask(std::make_shared<detail::ParallelCollectTask<TDATA>>(mManager, mNbTimes, onFinishOp));
    mTasks.reserve(mNbTimes + 1);
    mTasks.emplace_back(terminalTask);

    auto result = terminalTask->result();

    for(size_t idx = 0; idx < mNbTimes; ++idx)
    {
        auto callback = [terminalTask, idx](typename detail::IParallelTask<TDATA>::VariantType result)->void
        {
            terminalTask->notifyCompletion(idx, std::move(result));
        };
        auto op = std::bind(mOp, idx, callback);
        auto task = std::make_shared<detail::ParallelTask<TDATA>>(mManager, op, terminalTask);
        mTasks.emplace_back(task);
        mManager->run(task);
    }

    return result;   
}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelFor<TDATA>::cancel()
{
    for(auto task : mTasks)
    {
        task->cancel();
    }
}

}
}