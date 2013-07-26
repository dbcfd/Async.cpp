#pragma once
#include "async_cpp/async/detail/ParallelCollectTask.h"

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Parallel running task
 */
template<class TDATA, class TRESULT=TDATA>
class ParallelTask : public IParallelTask {
public:
    typedef typename std::function<void(OpResult<TRESULT>&&)> callback_t;
    typedef typename std::function<void(callback_t)> operation_t;

    ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        const size_t taskOrder,
        std::shared_ptr<ParallelCollectTask<TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();

protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;
    virtual void notifyException(const std::exception& ex) final;

private:
    typename operation_t mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask<TRESULT>> mCollectTask;
    size_t mTaskOrder;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelTask<TDATA, TRESULT>::ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        const size_t taskOrder,
        std::shared_ptr<ParallelCollectTask<TRESULT>> collectTask)
    : IParallelTask(mgr), mGenerateResultFunc(std::move(generateResult)), mCollectTask(collectTask), mTaskOrder(taskOrder)
{
    if(!mCollectTask) { throw(std::invalid_argument("ParallelTask: No collect task")); }
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
ParallelTask<TDATA, TRESULT>::~ParallelTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelTask<TDATA, TRESULT>::performSpecific()
{
    auto callback = [this](OpResult<TRESULT>&& result)->void {
        mCollectTask->notifyCompletion(mTaskOrder, std::move(result));
    };
    mGenerateResultFunc(callback);
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelTask<TDATA, TRESULT>::notifyCancel()
{
    mCollectTask->notifyCompletion(mTaskOrder, OpResult<TRESULT>(std::string("Cancelled")));
}

//------------------------------------------------------------------------------
template<class TDATA, class TRESULT>
void ParallelTask<TDATA, TRESULT>::notifyException(const std::exception& ex)
{
    mCollectTask->notifyCompletion(mTaskOrder, OpResult<TRESULT>(std::string(ex.what())));
}

}
}
}