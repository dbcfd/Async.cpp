#pragma once
#include "async_cpp/async/detail/ParallelTask.h"

namespace async_cpp {
namespace async {

/**
 * Perform an operation in parallel against all data in a vector, optionally calling a function to examine all results once parallel operations are complete.
 */
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT=TDATA>
class ParallelForEach {
public:
    typedef typename std::function<void(TDATA&, typename detail::ParallelTask<TRESULT>::callback_t)> operation_t;
    typedef typename typename detail::ParallelTask<TRESULT>::callback_t callback_t;
    typedef typename typename detail::ParallelCollectTask<TRESULT>::then_t then_t;
    typedef typename typename detail::ParallelCollectTask<TRESULT>::result_set_t result_set_t;
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    ParallelForEach(tasks::ManagerPtr manager, 
        typename operation_t op, 
        std::vector<TDATA>&& data);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return AsyncResult that holds a future completion status, either successful or exception
     */
    AsyncResult then(typename then_t onFinishTask);

    /**
     * Cancel outstanding tasks
     */
    void cancel();

private:
    typename operation_t mOp;
    tasks::ManagerPtr mManager;
    std::vector<std::shared_ptr<detail::IParallelTask<TRESULT>>> mTasks;
    std::vector<TDATA> mData;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelForEach<TDATA, TRESULT>::ParallelForEach(tasks::ManagerPtr manager, 
        typename operation_t op, 
        std::vector<TDATA>&& data)
    : mManager(manager), mData(std::move(data)), mOp(op)
{
    if(!mManager) { throw(std::invalid_argument("ParallelForEach: Manager cannot be null")); }
    if(mData.empty()) { throw(std::invalid_argument("ParallelForEach: Data cannot be empty")); }    
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
AsyncResult ParallelForEach<TDATA, TRESULT>::then(typename detail::ParallelCollectTask<TRESULT>::then_t onFinishOp)
{
    auto terminalTask(std::make_shared<detail::ParallelCollectTask<TRESULT>>(mManager, mData.size(), onFinishOp));
    mTasks.reserve(mData.size() + 1);
    mTasks.emplace_back(terminalTask);

    auto result = terminalTask->result();

    for(size_t i = 0; i < mData.size(); ++i)
    {
        mTasks.emplace_back(std::make_shared<detail::ParallelTask<TRESULT>>(mManager, std::bind(mOp, std::move(mData[i]), std::placeholders::_1), i, terminalTask));
        mManager->run(mTasks.back());
    }
    mData.clear();

    return result;
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelForEach<TDATA, TRESULT>::cancel()
{
    for(auto task : mTasks)
    {
        task->cancel();
    }
}

}
}