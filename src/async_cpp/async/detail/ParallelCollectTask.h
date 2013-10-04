#pragma once
#include "async_cpp/async/detail/IParallelTask.h"
#include "async_cpp/async/detail/ReadyVisitor.h"
#include "async_cpp/async/detail/ValueVisitor.h"
#include "async_cpp/tasks/IManager.h"

#include <boost/variant.hpp>
#include <functional>
#include <map>

namespace async_cpp
{
namespace async
{
namespace detail
{

/**
 * Task which collects a set of futures from parallel tasks.
 */
//------------------------------------------------------------------------------
template<class TRESULT>
class ParallelCollectTask : public IParallelTask<TRESULT>
{
public:
    typedef typename std::vector<TRESULT> result_set_t;
    typedef typename std::function < void(std::exception_ptr, result_set_t&& ) > then_t;
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
                        const size_t tasksOutstanding,
                        typename then_t thenFunction);
    ParallelCollectTask(ParallelCollectTask&& other);
    virtual ~ParallelCollectTask();

    AsyncResult result();
    void notifyCompletion(const size_t taskOrder, typename VariantType&& result);
    virtual void notifyException(std::exception_ptr ex) final;

protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;

private:
    std::mutex mResultsMutex;
    std::map<size_t, typename VariantType> mResults;
    size_t mResultsRequired;
    result_set_t mPreparedResults;
    std::packaged_task<bool(std::exception_ptr, result_set_t&&)> mTask;
    std::atomic_bool mValid;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
ParallelCollectTask<TRESULT>::ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding,
        typename then_t thenFunction)
    : IParallelTask(mgr),
      mResultsRequired(tasksOutstanding)
{
    mValid.store(true);
    mPreparedResults.reserve(mResultsRequired);
    mTask = std::packaged_task<bool(std::exception_ptr, result_set_t&&)>([thenFunction](std::exception_ptr ex, result_set_t&& results)->bool
    {
        try
        {
            thenFunction(ex, std::move(results));
        }
        catch(...)
        {
            ex = std::current_exception();
        }


        if(ex) std::rethrow_exception(ex);

        return true;
    } );
}

//------------------------------------------------------------------------------
template<class TRESULT>
ParallelCollectTask<TRESULT>::ParallelCollectTask(ParallelCollectTask&& other)
    : IParallelTask(std::move(other)),
      mResultsRequired(std::move(other.mResultsRequired)),
      mResults(std::move(other.mResults)),
      mPreparedResults(std::move(other.mPreparedResults)),
      mTask(std::move(other.mTask))
{
    mValid.store(other.mValid);
}

//------------------------------------------------------------------------------
template<class TRESULT>
ParallelCollectTask<TRESULT>::~ParallelCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelCollectTask<TRESULT>::performSpecific()
{
    if(mValid)
    {
        ReadyVisitor<TRESULT> isReady;
        ValueVisitor<TRESULT> getValue;
        std::map<size_t, typename VariantType> readyResults;
        for(auto& kv : mResults)
        {
            if(boost::apply_visitor(isReady, kv.second))
            {
                auto resultPtr = boost::apply_visitor(getValue, kv.second);
                if(resultPtr)
                {
                    mPreparedResults.push_back(std::move(*resultPtr));
                }
            }
            else
            {
                readyResults.insert(std::move(kv));
            }
        }
        mResults.clear();
        mResults.swap(readyResults);

        if(mResults.empty())
        {
            mTask(nullptr, std::move(mPreparedResults));
            mPreparedResults.clear();
        }
        else
        {
            mManager.lock()->run(std::make_shared<ParallelCollectTask>(std::move(*this)));
        }
    }
}

//------------------------------------------------------------------------------
template<class TRESULT>
AsyncResult ParallelCollectTask<TRESULT>::result()
{
    return AsyncResult(mTask.get_future());
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelCollectTask<TRESULT>::notifyCompletion(const size_t taskIndex, typename VariantType&& result)
{
    //only care about results if we're still valid
    if(mValid)
    {
        std::lock_guard<std::mutex> lock(mResultsMutex);
        //prevent double callbacks
        if(mResults.find(taskIndex) == mResults.end())
        {
            mResults.emplace(taskIndex, std::move(result));
            if(mResults.size() == mResultsRequired)
            {
                mManager.lock()->run(shared_from_this());
            }
        }
    }
}

//------------------------------------------------------------------------------
template<class T>
void ParallelCollectTask<T>::notifyCancel()
{
    //mark as invalid, so we won't attempt to run this task
    mValid.exchange(false);
    mTask(std::make_exception_ptr(std::runtime_error("Cancelled")), result_set_t());
}

//------------------------------------------------------------------------------
template<class T>
void ParallelCollectTask<T>::notifyException(std::exception_ptr ex)
{
    mValid.exchange(false);
    mTask(ex, result_set_t());
}

}
}
}