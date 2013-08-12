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
    typedef typename std::function<void(typename VariantType)> callback_t;
    typedef typename std::function<void(callback_t)> operation_t;

    ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        const size_t taskOrder,
        std::shared_ptr<ParallelCollectTask<TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();
    virtual void notifyException(std::exception_ptr ex) final;

protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;

private:
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
    auto thisPtr = shared_from_this();
    auto callback = [thisPtr, this](typename VariantType result)->void {
        mCollectTask->notifyCompletion(mTaskOrder, std::move(result));
    };
    mGenerateResultFunc(callback);
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::notifyCancel()
{
    mCollectTask->cancel();
}

//------------------------------------------------------------------------------
template<class TRESULT>
void ParallelTask<TRESULT>::notifyException(std::exception_ptr ex)
{
    mCollectTask->notifyException(ex);
}

}
}
}