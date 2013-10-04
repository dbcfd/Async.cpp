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

    ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<void(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TRESULT>> parallelCollectTask);
    virtual ~ParallelTask();
    virtual void notifyException(std::exception_ptr ex) final;

protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;

private:
    std::function<void(void)> mGenerateResultFunc;
    std::shared_ptr<ParallelCollectTask<TRESULT>> mCollectTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
ParallelTask<TRESULT>::ParallelTask(std::weak_ptr<tasks::IManager> mgr, 
        std::function<void(void)> generateResult,
        std::shared_ptr<ParallelCollectTask<TRESULT>> collectTask)
    : IParallelTask(mgr), mGenerateResultFunc(generateResult), mCollectTask(collectTask)
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
    mGenerateResultFunc();
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