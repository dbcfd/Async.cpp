#pragma once
#include "async/Platform.h"
#include "async/AsyncResult.h"
#include "async/ParallelTask.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>
#include <vector>

namespace async_cpp {

namespace workers {
class IManager;
}

namespace async {

/**
 * Perform an operation in parallel against all data in a vector, optionally calling a function to examine all results once parallel operations are complete.
 */
template<class TVOID>
class ParallelForEachTemplate {
public:
    /**
     * Create a parallel task set using a manager and a set of tasks.
     * @param manager Manager to run tasks against
     * @param tasks Vector of tasks that will be run
     */
    ParallelForEachTemplate(std::shared_ptr<workers::IManager> manager, 
        std::function<AsyncFuture(std::shared_ptr<TVOID>)> op, 
        const std::vector<std::shared_ptr<TVOID>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<AsyncFuture(std::vector<AsyncResult>&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    std::function<AsyncFuture(std::shared_ptr<TVOID>)> mOp;
    std::shared_ptr<workers::IManager> mManager;
    std::vector<std::shared_ptr<TVOID>> mData;
};

//------------------------------------------------------------------------------
typedef ParallelForEachTemplate<void> ParallelForEach;
typedef ParallelForEachTemplate<const void> ParallelForEachConst;

//inline implementations
//------------------------------------------------------------------------------
template<class TVOID>
ParallelForEachTemplate<TVOID>::ParallelForEachTemplate(std::shared_ptr<workers::IManager> manager, 
        std::function<AsyncFuture(std::shared_ptr<TVOID>)> op, 
        const std::vector<std::shared_ptr<TVOID>>& data)
    : mManager(manager), mData(data), mOp(op)
{
    assert(manager);
    assert(!data.empty());
    
}

//------------------------------------------------------------------------------
template<class TVOID>
AsyncFuture ParallelForEachTemplate<TVOID>::execute(std::function<AsyncFuture(std::vector<AsyncResult>&)> onFinishOp)
{
    std::shared_ptr<ParallelCollectTask> terminalTask(new ParallelCollectTask(mManager, mData.size(), onFinishOp));

    auto future = terminalTask->getFuture();

    for(auto data : mData)
    {
        auto op = [this, data](void) -> AsyncFuture {
            return mOp(data);
        };
        mManager->run(std::shared_ptr<workers::Task>(new ParallelTask(mManager, op, terminalTask)));
    }

    return future;
}

//------------------------------------------------------------------------------
template<class TVOID>
AsyncFuture ParallelForEachTemplate<TVOID>::execute()
{
    return execute([](std::vector<AsyncResult>& input)->AsyncFuture { 
        return AsyncResult().asFulfilledFuture();
    });
}

}
}