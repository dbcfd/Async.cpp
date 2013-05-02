#include "async/Parallel.h"
#include "async/AsyncResult.h"
#include "async/ParallelTask.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<AsyncFuture(void)>>& tasks)
    : mManager(manager), mOps(tasks)
{
    assert(nullptr != manager);
    assert(!tasks.empty());
}

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, std::function<AsyncFuture(void)> tasks[], const size_t nbTasks)
    : mManager(manager)
{
    assert(nullptr != manager);
    assert(nullptr != tasks);

    mOps.assign(tasks, tasks+nbTasks);

    assert(!mOps.empty());
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute(std::function<AsyncFuture(std::vector<AsyncResult>&)> onFinishOp)
{
    auto terminalTask(std::make_shared<ParallelCollectTask>(mManager, mOps.size(), onFinishOp));

    auto future = terminalTask->getFuture();

    for(auto op : mOps)
    {
        mManager->run(std::make_shared<ParallelTask>(mManager, op, terminalTask));
    }

    return future; 
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute()
{
    return execute([](std::vector<AsyncResult>& in)->AsyncFuture { 
        return AsyncResult().asFulfilledFuture();
    } ); 
}

}
}
