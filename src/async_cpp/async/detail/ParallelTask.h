#pragma once
#include "async_cpp/async/detail/ParallelCollectTask.h"

#include <boost/variant.hpp>

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Parallel running task
 */
//------------------------------------------------------------------------------
template<class TRESULT>
class ParallelTask : public IParallelTask<TRESULT> {
public:
    typedef typename std::function<void(typename VariantType&&)> callback_t;
    typedef typename std::function<void(callback_t)> operation_t;

    ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        const size_t taskOrder,
        std::shared_ptr<ParallelCollectTask<TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;
    virtual void notifyException(std::exception_ptr ex) final;

private:
    virtual void attemptOperation(std::function<void(void)> func) final;

    std::shared_ptr<ParallelCollectTask<TRESULT>> mCollectTask;
    typename operation_t mGenerateResultFunc;
    size_t mTaskOrder;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
ParallelTask<TRESULT>::ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        const size_t taskOrder,
        std::shared_ptr<ParallelCollectTask<TRESULT>> collectTask)
    : IParallelTask(mgr), mGenerateResultFunc(generateResult), mCollectTask(collectTask), mTaskOrder(taskOrder)
{
    if(!mCollectTask) { throw(std::invalid_argument("ParallelTask: No collect task")); }

}

//------------------------------------------------------------------------------
template<class TRESULT>
ParallelTask<TRESULT>::~ParallelTask()
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::performSpecific()
{
    auto callback = [this](typename VariantType&& result)->void {
        mCollectTask->notifyCompletion(mTaskOrder, std::move(result));
    };
    attemptOperation([callback, this]()->void {
        mGenerateResultFunc(callback);
    } );
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::attemptOperation(std::function<void(void)> func)
{
    try
    {
        func();
    }
    catch(...)
    {
        mCollectTask->notifyCompletion(mTaskOrder, std::current_exception());
    }
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::notifyCancel()
{
    mCollectTask->cancel();
    attemptOperation([]()->void {
        throw(std::runtime_error("Cancelled"));
    } );
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::notifyException(std::exception_ptr ex)
{
    attemptOperation([ex]()->void {
        std::rethrow_exception(ex);
    } );
}

}
}
}