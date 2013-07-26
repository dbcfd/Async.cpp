#pragma once
#include "async_cpp/async/detail/IParallelTask.h"
#include "async_cpp/async/AsyncResult.h"
#include "async_cpp/async/OpResult.h"

#include <functional>
#include <future>
#include <map>
#include <vector>

namespace async_cpp
{
namespace async
{
namespace detail
{

/**
 * Task which collects a set of futures from parallel tasks.
 */
template<class TDATA>
class ParallelCollectTask : public IParallelTask
{
public:
    typedef typename std::vector<OpResult<TDATA>> result_set_t;
    typedef typename std::function < void(AsyncResult&&) > callback_t;
    typedef typename std::function < void(OpResult<result_set_t>&&, typename callback_t ) > then_t;
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
                        const size_t tasksOutstanding,
                        typename then_t thenFunction);
    virtual ~ParallelCollectTask();

    std::future<AsyncResult> future();
    void notifyCompletion(const size_t taskOrder, OpResult<TDATA>&& result);
protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;
    virtual void notifyException(const std::exception& ex);

private:
    typename then_t mThenFunction;
    size_t mTasksOutstanding;
    std::mutex mResultsMutex;
    std::map<size_t, OpResult<TDATA>> mResults;
    std::promise<AsyncResult> mPromise;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
ParallelCollectTask<TDATA>::ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding,
        typename then_t thenFunction)
    : IParallelTask(mgr),
      mTasksOutstanding(tasksOutstanding),
      mThenFunction(thenFunction)
{

}

//------------------------------------------------------------------------------
template<class TDATA>
ParallelCollectTask<TDATA>::~ParallelCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelCollectTask<TDATA>::performSpecific()
{
    result_set_t results;
    results.reserve(mResults.size());

    auto callback = [this](AsyncResult&& result)->void
    {
        mPromise.set_value(std::move(result));
    };

    for(auto & kv : mResults)
    {
        auto& result = kv.second;
        if(result.wasError())
        {
            mThenFunction(OpResult<result_set_t>(result.error()), callback);
            return;
        }
        results.push_back(std::move(result));
    }

    mResults.clear();
    mThenFunction(OpResult<result_set_t>(std::move(results)), callback);
}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelCollectTask<TDATA>::notifyCancel()
{
    mPromise.set_value(AsyncResult(std::string("Cancelled")));
}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelCollectTask<TDATA>::notifyException(const std::exception& ex)
{
    mPromise.set_value(AsyncResult(ex.what()));
}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult> ParallelCollectTask<TDATA>::future()
{
    return mPromise.get_future();
}

//------------------------------------------------------------------------------
template<class TDATA>
void ParallelCollectTask<TDATA>::notifyCompletion(const size_t taskIndex, OpResult<TDATA>&& result)
{
    std::lock_guard<std::mutex> lock(mResultsMutex);
    mResults.emplace(taskIndex, std::move(result));
    if(mResults.size() == mTasksOutstanding)
    {
        mManager.lock()->run(shared_from_this());
    }
}

}
}
}