#include "async_cpp/async/ParallelFor.h"
#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/ParallelTask.h"

#include "async_cpp/tasks/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelFor::ParallelFor(std::shared_ptr<tasks::IManager> manager, 
        std::function<AsyncFuture(size_t)> op, 
        const size_t nbTimes)
    : mManager(manager), mOp(op), mNbTimes(nbTimes)
{
    assert(nullptr != manager);
    assert(0 < nbTimes);
    
}

//------------------------------------------------------------------------------
AsyncFuture ParallelFor::execute(std::function<AsyncFuture(std::vector<AsyncResult>&)> onFinishOp)
{
    auto terminalTask(std::make_shared<ParallelCollectTask>(mManager, mNbTimes, onFinishOp));

    auto future = terminalTask->getFuture();

    for(size_t idx = 0; idx < mNbTimes; ++idx)
    {
        auto op = std::bind(mOp, idx);
        mManager->run(std::make_shared<ParallelTask>(mManager, op, terminalTask));
    }

    return future;   
}

//------------------------------------------------------------------------------
AsyncFuture ParallelFor::execute()
{
    return execute([](std::vector<AsyncResult>& input)->AsyncFuture { 
        return AsyncResult().asFulfilledFuture();
    } );
}

}
}
