#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ParallelTask.h"

namespace async_cpp {
namespace async {

/**
 * Perform an operation in parallel against all data in a vector, optionally calling a function to examine all results once parallel operations are complete.
 */
template<class TIN, class TOUT, class TRESULT = TOUT>
class ParallelForEach {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    ParallelForEach(std::shared_ptr<tasks::IManager> manager, 
        std::function<std::future<AsyncResult<TOUT>>(std::shared_ptr<TIN>)> op, 
        const std::vector<std::shared_ptr<TIN>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    std::future<AsyncResult<TRESULT>> execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TOUT>>&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
    std::future<AsyncResult<TRESULT>> execute();

private:
    std::function<std::future<AsyncResult<TOUT>>(std::shared_ptr<TIN>)> mOp;
    std::shared_ptr<tasks::IManager> mManager;
    std::vector<std::shared_ptr<TIN>> mData;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TIN, class TOUT, class TRESULT>
ParallelForEach<TIN, TOUT, TRESULT>::ParallelForEach(std::shared_ptr<tasks::IManager> manager, 
        std::function<std::future<AsyncResult<TOUT>>(std::shared_ptr<TIN>)> op, 
        const std::vector<std::shared_ptr<TIN>>& data)
    : mManager(manager), mData(data), mOp(op)
{
    assert(manager);
    assert(!data.empty());
    
}

//------------------------------------------------------------------------------
template<class TIN, class TOUT, class TRESULT>
std::future<AsyncResult<TRESULT>> ParallelForEach<TIN, TOUT, TRESULT>::execute(std::function<std::future<AsyncResult<TRESULT>>(const std::vector<AsyncResult<TOUT>>&)> onFinishOp)
{
    auto terminalTask(std::make_shared<ParallelCollectTask<TOUT, TRESULT>>(mManager, mData.size(), onFinishOp));

    auto future = terminalTask->getFuture();

    for(auto data : mData)
    {
        mManager->run(std::make_shared<ParallelTask<TOUT>>(mManager, std::bind(mOp, data), terminalTask));
    }

    return future;
}

//------------------------------------------------------------------------------
template<class TIN, class TOUT, class TRESULT>
std::future<AsyncResult<TRESULT>> ParallelForEach<TIN, TOUT, TRESULT>::execute()
{
    return execute([](const std::vector<AsyncResult<TOUT>>& input)->std::future<AsyncResult<TOUT>> { 
        return AsyncResult<TRESULT>().asFulfilledFuture();
    });
}

}
}