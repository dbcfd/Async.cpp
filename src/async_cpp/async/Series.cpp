#include "async/Series.h"
#include "async/AsyncResult.h"
#include "async/SeriesTask.h"

#include "workers/IManager.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<AsyncFuture(AsyncResult&)>>& ops)
    : mManager(manager), mOperations(ops)
{
    assert(manager);
    assert(!ops.empty());
}

//------------------------------------------------------------------------------
Series::Series(std::shared_ptr<workers::IManager> manager, std::function<AsyncFuture(AsyncResult&)>* ops, const size_t nbOps)
    : mManager(manager)
{
    assert(manager);
    assert(ops);

    mOperations.assign(ops, ops+nbOps);
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute(std::function<AsyncFuture(AsyncResult&)> onFinishOp)
{
    auto finishTask(std::make_shared<SeriesCollectTask>(mManager, onFinishOp));

    auto result = finishTask->getFuture();

    std::shared_ptr<ISeriesTask> nextTask = finishTask;
    for(auto opItr = mOperations.rbegin(); opItr != mOperations.rend(); ++opItr)
    {
        nextTask = std::make_shared<SeriesTask>(mManager, (*opItr), nextTask);
    }

    mManager->run(nextTask);

    return result;  
}

//------------------------------------------------------------------------------
AsyncFuture Series::execute()
{
    return execute([](AsyncResult& in)->AsyncFuture { 
        return AsyncResult().asFulfilledFuture();
    });
}

}
}
