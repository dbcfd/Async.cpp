#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelTask.h"

namespace async_cpp {
namespace async {

/**
 * Perform an operation in parallel for a number of times, optionally calling a function to examine all results once parallel 
 * operations are complete. Each task will be passed an index as data.
 */
template<class TDATA, class TRESULT = TDATA>
class ParallelFor {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param op Operation to run in parallel for a number of times
     * @param nbTimes Number of times to run operation for
     */
    ParallelFor(std::shared_ptr<tasks::IManager> manager, 
        std::function<std::future<AsyncResult<TDATA>>(size_t)> op, 
        const size_t nbTimes);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
    std::future<AsyncResult<TRESULT>> execute();

private:
    std::function<std::future<AsyncResult<TDATA>>(size_t)> mOp;
    std::shared_ptr<tasks::IManager> mManager;
    size_t mNbTimes;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelFor<TDATA, TRESULT>::ParallelFor(std::shared_ptr<tasks::IManager> manager, 
        std::function<std::future<AsyncResult<TDATA>>(size_t)> op, 
        const size_t nbTimes)
    : mManager(manager), mOp(op), mNbTimes(nbTimes)
{
    assert(nullptr != manager);
    assert(0 < nbTimes);
    
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> ParallelFor<TDATA, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TDATA>>&)> onFinishOp)
{
    auto terminalTask(std::make_shared<ParallelCollectTask<TDATA,TRESULT>>(mManager, mNbTimes, onFinishOp));

    auto future = terminalTask->getFuture();

    for(size_t idx = 0; idx < mNbTimes; ++idx)
    {
        auto op = std::bind(mOp, idx);
        mManager->run(std::make_shared<ParallelTask<TDATA, TRESULT>>(mManager, op, terminalTask));
    }

    return future;   
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
std::future<AsyncResult<TRESULT>> ParallelFor<TDATA, TRESULT>::execute()
{
    return execute([](const std::vector<AsyncResult<TDATA>>& input)->std::future<AsyncResult<TRESULT>> { 
        return AsyncResult<TRESULT>().asFulfilledFuture();
    } );
}

}
}