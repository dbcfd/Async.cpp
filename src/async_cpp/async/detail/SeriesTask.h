#pragma once
#include "async_cpp/async/detail/ISeriesTask.h"

namespace async_cpp {
namespace async {
namespace detail {

/**
 * Task which continues a chain of asynchronous tasks.
 */
template<class TRESULT>
class SeriesTask : public ISeriesTask<TRESULT> {
public:
    typedef typename std::function<void(typename VariantType&&)> callback_t;
    typedef typename std::function<void(std::exception_ptr, TRESULT*, typename callback_t)> operation_t;
    /**
     * Create an asynchronous task that does not take in information and returns an AsyncResult via a packaged_task.
     * @param generateResult packaged_task that will produce the AsyncResult
     */
    SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        std::shared_ptr<ISeriesTask<TRESULT>> nextTask);    
    virtual ~SeriesTask();

protected:
    virtual void performSpecific() final;

private:
    virtual void attemptOperation(std::function<void(void)> op) final;

    std::shared_ptr<ISeriesTask<TRESULT>> mNextTask;
    typename operation_t mGenerateResultFunc;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
SeriesTask<TRESULT>::SeriesTask(std::weak_ptr<tasks::IManager> mgr, 
        typename operation_t generateResult,
        std::shared_ptr<ISeriesTask<TRESULT>> nextTask)
    : ISeriesTask<TRESULT>(mgr), mNextTask(nextTask), mGenerateResultFunc(generateResult)
{
    if(!mNextTask) { throw(std::runtime_error("SeriesTask: No next task")); }
}

//------------------------------------------------------------------------------
template<class TRESULT>
SeriesTask<TRESULT>::~SeriesTask()
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
void SeriesTask<TRESULT>::performSpecific()
{
    auto callback = [this](typename VariantType&& result)->void {
        mNextTask->begin(std::move(result));
    };

    attemptOperation([this, callback]()->void {
        mGenerateResultFunc(nullptr, boost::apply_visitor(ValueVisitor<TRESULT>(), mPreviousResult), callback);
    } );
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTask<TDATA>::attemptOperation(std::function<void(void)> op)
{
    try
    {
        op();
    }
    catch(...)
    {
        mNextTask->begin(std::current_exception());
    }
}

}
}
}