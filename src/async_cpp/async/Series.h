#pragma once
#include "async_cpp/async/detail/SeriesCollectTask.h"
#include "async_cpp/async/detail/SeriesTask.h"

#include <functional>

namespace async_cpp {
namespace async {

/**
 * Run a set of tasks in series using a manager, optionally executing a task when the set of parallel tasks is complete.
 * A future is created which indicates when this set of operations (including optional completion task) is completed.
 */
template<class TDATA>
class Series {
public:
    typedef typename typename detail::SeriesTask<TDATA>::operation_t operation_t;
    typedef typename typename detail::SeriesTask<TDATA>::callback_t callback_t;
    typedef typename typename detail::SeriesCollectTask<TDATA>::then_t then_t;
    typedef typename typename detail::SeriesCollectTask<TDATA>::callback_t complete_t;
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Vector of tasks that will be run
     */
    Series(tasks::ManagerPtr manager, 
        const std::vector<typename operation_t>& ops);
    /**
     * Create a series task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param ops Array of operations that will be run
     * @param nbOps Number of operations in array
     */
    Series(tasks::ManagerPtr manager, 
        typename operation_t ops[], const size_t nbOps);

    /**
     * Run the set of tasks in series, calling a task when the series tasks have completed.
     * @param onFinishTask Task to run when series tasks are complete
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult> then(typename then_t onFinishTask);

    /**
     * Cancel outstanding tasks
     */
    void cancel();

private:
    std::vector<typename operation_t> mOperations;
    tasks::ManagerPtr mManager;
    std::vector<std::shared_ptr<async::detail::ISeriesTask<TDATA>>> mTasks;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
Series<TDATA>::Series(tasks::ManagerPtr manager, const std::vector<typename detail::SeriesTask<TDATA>::operation_t>& ops)
    : mManager(manager), mOperations(ops)
{
    if(!mManager) { throw(std::invalid_argument("Series: Manager cannot be null")); }
    if(mOperations.empty()) { throw(std::invalid_argument("Series: Ops cannot be empty")); }
}

//------------------------------------------------------------------------------
template<class TDATA>
Series<TDATA>::Series(tasks::ManagerPtr manager, typename detail::SeriesTask<TDATA>::operation_t ops[], const size_t nbOps)
    : mManager(manager)
{
    if(!mManager) { throw(std::invalid_argument("Series: Manager cannot be null")); }
    if(!ops) { throw(std::invalid_argument("Series: Ops must be defined")); }

    mOperations.assign(ops, ops+nbOps);
}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult> Series<TDATA>::then(typename detail::SeriesCollectTask<TDATA>::then_t onFinishOp)
{
    auto finishTask(std::make_shared<detail::SeriesCollectTask<TDATA>>(mManager, onFinishOp));
    mTasks.reserve(mOperations.size() + 1);
    mTasks.emplace_back(finishTask);

    auto result = finishTask->future();

    std::shared_ptr<detail::ISeriesTask<TDATA>> nextTask = finishTask;
    for(auto iter = mOperations.rbegin(); iter != mOperations.rend(); ++iter)
    {
        nextTask = std::make_shared<detail::SeriesTask<TDATA>>(mManager, (*iter), nextTask);
        mTasks.emplace_back(nextTask);
    }

    mManager->run(nextTask);

    return result;  
}

//------------------------------------------------------------------------------
template<class TDATA>
void Series<TDATA>::cancel()
{
    for(auto task : mTasks)
    {
        task->cancel();
    }
}

}
}