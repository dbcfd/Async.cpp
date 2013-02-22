#pragma once
#include "async/Platform.h"
#include "async/AsyncTask.h"
#include "async/AsyncResult.h"

#include "workers/IManager.h"

#include <assert.h>
#include <atomic>

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
        std::function<AsyncResult(std::shared_ptr<TVOID>)> op, 
        const std::vector<std::shared_ptr<TVOID>>& data);

    /**
     * Run the operation across the set of data, invoking a task with the result of the data
     * @param onFinishTask Task to run when operation has been applied to all data
     * @return Future indicating when all operations (including onFinishTask) are complete
     */
    AsyncFuture execute(std::function<AsyncResult(AsyncResult&)> onFinishTask);
    /**
     * Run the operation across the set of data.
     * @return Future indicating when all operations are complete
     */
    AsyncFuture execute();

private:
    std::function<AsyncResult(std::shared_ptr<TVOID>)> mOp;
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
        std::function<AsyncResult(std::shared_ptr<TVOID>)> op, 
        const std::vector<std::shared_ptr<TVOID>>& data)
    : mManager(manager), mData(data), mOp(op)
{
    assert(nullptr != manager);
    assert(!data.empty());
    
}

//------------------------------------------------------------------------------
template<class TVOID>
AsyncFuture ParallelForEachTemplate<TVOID>::execute(std::function<AsyncResult(AsyncResult&)> onFinishOp)
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
            [data, opsRemaining, finishTask](std::function<AsyncResult(std::shared_ptr<TVOID>)> op, std::shared_ptr<workers::IManager> mgr)->void {
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
template<class TVOID>
AsyncFuture ParallelForEachTemplate<TVOID>::execute()
{
    return execute([](AsyncResult& input)->AsyncResult { return std::move(input); });
}

}
}