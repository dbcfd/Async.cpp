#pragma once
#include "async_cpp/async/detail/IParallelTask.h"
#include "async_cpp/async/detail/ReadyVisitor.h"
#include "async_cpp/async/detail/ValueVisitor.h"

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
protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;
    virtual void notifyException(std::exception_ptr ex) final;

private:
    virtual void attemptOperation(std::function<void(void)> op) final;

    typename then_t mThenFunction;
    size_t mTasksOutstanding;
    std::mutex mResultsMutex;
    std::map<size_t, VariantType> mResults;
    result_set_t mPreparedResults;
    std::promise<bool> mPromise;
    std::atomic_bool mValid;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
ParallelCollectTask<TRESULT>::ParallelCollectTask(std::weak_ptr<tasks::IManager> mgr,
        const size_t tasksOutstanding,
        typename then_t thenFunction)
    : IParallelTask(mgr),
      mTasksOutstanding(tasksOutstanding),
      mThenFunction(thenFunction)
{
    mValid.store(true);
    mPreparedResults.reserve(mTasksOutstanding);
}

//------------------------------------------------------------------------------
template<class TRESULT>
ParallelCollectTask<TRESULT>::ParallelCollectTask(ParallelCollectTask&& other)
    : IParallelTask(std::move(other)),
      mTasksOutstanding(std::move(other.mTasksOutstanding)),
      mThenFunction(std::move(other.mThenFunction)),
      mResults(std::move(other.mResults)),
      mPromise(std::move(other.mPromise)),
      mPreparedResults(std::move(other.mPreparedResults))
{
    mValid.store(true);
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
    attemptOperation([this]()->void {
        ReadyVisitor<TRESULT> isReady;
        ValueVisitor<TRESULT> getValue;
        bool allResultsReady = true;
        auto iter = mResults.begin();
        for(iter; allResultsReady && iter != mResults.end(); ++iter)
        {
            auto& varResult = iter->second;
            if(boost::apply_visitor(isReady, varResult))
            {
                auto resultPtr = boost::apply_visitor(getValue, varResult);
                if(resultPtr)
                {
                    mPreparedResults.push_back(std::move(*resultPtr));
                }
            }
            else
            {
                allResultsReady = false;
            }
        }
        mResults.erase(mResults.begin(), iter);

        if(allResultsReady)
        {
            try
            {
                mThenFunction(nullptr, std::move(mPreparedResults));
                mPromise.set_value(true);
            }
            catch(...)
            {
                mPromise.set_exception(std::current_exception());
            }
        }
        else
        {
            mManager.lock()->run(std::make_shared<ParallelCollectTask>(std::move(*this)));
        }
    } );
}



//------------------------------------------------------------------------------
template<class TRESULT>
AsyncResult ParallelCollectTask<TRESULT>::result()
{
    return AsyncResult(mPromise.get_future());
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelCollectTask<TRESULT>::notifyCompletion(const size_t taskIndex, typename VariantType&& result)
{
    std::lock_guard<std::mutex> lock(mResultsMutex);
    mResults.emplace(taskIndex, std::move(result));
    if(mResults.size() == mTasksOutstanding)
    {
        //only want to run this task if not cancelled, otherwise, mark as cancelled once all other tasks finish
        if(mValid)
        {
            mManager.lock()->run(shared_from_this());
        }
        else
        {
            attemptOperation([]()->void {
                throw(std::runtime_error("Cancelled"));
            } );
        }
    }
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelCollectTask<TRESULT>::attemptOperation(std::function<void(void)> func)
{
    try
    {
        func();
    }
    catch(...)
    {
        try
        {
            mThenFunction(std::current_exception(), result_set_t());
            mPromise.set_exception(std::current_exception());
        }
        catch(...)
        {
            mPromise.set_exception(std::current_exception());
        }
    }
}

//------------------------------------------------------------------------------
template<class T>
void ParallelCollectTask<T>::notifyCancel()
{
    //mark as invalid, so we won't attempt to run this task
    mValid.exchange(false);
}

//------------------------------------------------------------------------------
template<class T>
void ParallelCollectTask<T>::notifyException(std::exception_ptr ex)
{
    attemptOperation([ex]()->void {
        std::rethrow_exception(ex);
    } );
}

}
}
}