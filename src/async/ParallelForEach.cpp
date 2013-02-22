#include "async/ParallelForEach.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
ParallelForEach::ParallelForEach(std::shared_ptr<workers::IManager> manager, 
        std::function<AsyncResult(std::shared_ptr<void>)> op, 
        const std::vector<std::shared_ptr<void>>& data)
    : mManager(manager), mData(data), mOp(op)
{
    assert(nullptr != manager);
    assert(!data.empty());
    
}

//------------------------------------------------------------------------------
AsyncFuture ParallelForEach::execute(std::function<AsyncResult(AsyncResult&)> onFinishOp)
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

    std::shared_ptr<std::atomic<size_t>> opsRemaining(new std::atomic<size_t>(mData.size()));

    size_t chunk = 0;
    std::vector<std::shared_ptr<workers::Task>> runningTasks;
    for(auto data : mData)
    {
        if(0 == chunk)
        {
            chunk = mManager->chunkSize();
            runningTasks.reserve(chunk);
        }
        if(runningTasks.size() == chunk)
        {
            for(auto task : runningTasks)
            {
                task->wasCompletedSuccessfully();
            }
            runningTasks.clear();
            chunk = mManager->chunkSize();
            runningTasks.reserve(chunk);
        }
        std::function<void(void)> func = std::bind(
            [data, opsRemaining, finishTask](std::function<AsyncResult(std::shared_ptr<void>)> op, std::shared_ptr<workers::IManager> mgr)->void {
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
        runningTasks.emplace_back(task);
        mManager->run(task);
    }

    return finishTask->future();   
}

//------------------------------------------------------------------------------
AsyncFuture ParallelForEach::execute()
{
    return execute([](AsyncResult& input)->AsyncResult { return std::move(input); });
}

}
}
