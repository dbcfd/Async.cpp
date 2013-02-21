#include "async/ParallelFor.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelFor::ParallelFor(std::shared_ptr<workers::IManager> manager, 
        std::function<AsyncResult(std::shared_ptr<void>)> op, 
        const size_t nbTimes)
    : mManager(manager), mOp(op), mNbTimes(nbTimes)
{
    assert(nullptr != manager);
    assert(0 < nbTimes);
    
}

//------------------------------------------------------------------------------
AsyncFuture ParallelFor::execute(std::function<AsyncResult(AsyncResult&)> onFinishOp)
{
    std::shared_ptr<AsyncTerminalTask> finishTask(new AsyncTerminalTask(std::function<AsyncResult(AsyncResult&)>(
        [onFinishOp](AsyncResult& input)-> AsyncResult {
            try 
            {
                input = std::move(onFinishOp(std::move(input)));
            }
            catch(...)
            {
                input = std::move(AsyncResult("Error in function invoked on finish"));
            }
            return input;
        }
    ) ) );

    std::shared_ptr<std::atomic<size_t>> opsRemaining(new std::atomic<size_t>(mNbTimes));

    for(size_t idx = 0; idx < mNbTimes; ++idx)
    {
        std::function<void(void)> func = std::bind(
            [idx, opsRemaining, finishTask](std::function<AsyncResult(std::shared_ptr<void>)> op, std::shared_ptr<workers::IManager> mgr)->void {
                std::shared_ptr<void> data(new size_t(idx));
                auto res = op(data);
                if(res.wasError())
                {
                    finishTask->forward(res);
                }
                if(1 == opsRemaining->fetch_sub(1))
                {
                    mgr->run(finishTask);
                }
            }, mOp, mManager);
        auto task = std::shared_ptr<AsyncTask>( new AsyncTask(func) );
        mManager->run(task);
    }

    return finishTask->future();   
}

//------------------------------------------------------------------------------
AsyncFuture ParallelFor::execute()
{
    return execute([](AsyncResult& input)->AsyncResult { return std::move(input); } );
}

}
}
