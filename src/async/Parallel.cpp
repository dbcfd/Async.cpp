#include "async/Parallel.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, const std::vector<std::function<AsyncResult(void)>>& tasks)
    : mManager(manager), mOps(tasks)
{
    assert(nullptr != manager);
    assert(!tasks.empty());
}

//------------------------------------------------------------------------------
Parallel::Parallel(std::shared_ptr<workers::IManager> manager, std::function<AsyncResult(void)> tasks[], const size_t nbTasks)
    : mManager(manager)
{
    assert(nullptr != manager);
    assert(nullptr != tasks);

    mOps.assign(tasks, tasks+nbTasks);

    assert(!mOps.empty());
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute(std::function<AsyncResult(AsyncResult&)> onFinishOp)
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

    std::shared_ptr<std::atomic<size_t>> opsRemaining(new std::atomic<size_t>(mOps.size()));

    for(auto op : mOps)
    {
        std::function<void(void)> func = std::bind(
            [op, opsRemaining, finishTask](std::shared_ptr<workers::IManager> mgr)->void {
                auto res = op();
                if(res.wasError())
                {
                    finishTask->forward(res);
                }
                if(1 == opsRemaining->fetch_sub(1))
                {
                    mgr->run(finishTask);
                }
            }, mManager);
        auto task = std::shared_ptr<AsyncTask>(new AsyncTask(func) );
        mManager->run(task);
    }

    return finishTask->future();  
}

//------------------------------------------------------------------------------
AsyncFuture Parallel::execute()
{
    return execute([](AsyncResult& in)->AsyncResult { return std::move(in); } );
}

}
}
