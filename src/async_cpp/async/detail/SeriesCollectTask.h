#pragma once
#include "async_cpp/async/detail/ISeriesTask.h"
#include "async_cpp/async/detail/ReadyVisitor.h"
#include "async_cpp/async/detail/ValueVisitor.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
template<class TRESULT>
class SeriesCollectTask : public ISeriesTask<TRESULT> {
public:
    typedef typename std::function<void(std::exception_ptr, TRESULT*)> then_t;
    SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr, typename then_t thenFunc);
    virtual ~SeriesCollectTask();

    AsyncResult result();
protected:
    virtual void performSpecific() final;

private:
    virtual void attemptOperation(std::function<void(void)> op) final;

    typename then_t mThenFunc;
    std::promise<bool> mPromise;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
SeriesCollectTask<TRESULT>::SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr,
                                     typename then_t thenFunc)
                                     : ISeriesTask<TRESULT>(mgr), 
                                     mThenFunc(thenFunc)
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
SeriesCollectTask<TRESULT>::~SeriesCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
void SeriesCollectTask<TRESULT>::performSpecific()
{
    attemptOperation([this]()->void {
        mThenFunc(nullptr, boost::apply_visitor(ValueVisitor<TRESULT>(), mPreviousResult));
        mPromise.set_value(true);
    } );
}

//------------------------------------------------------------------------------
template<class TRESULT>
AsyncResult SeriesCollectTask<TRESULT>::result()
{
    return AsyncResult(mPromise.get_future());
}

//------------------------------------------------------------------------------
template<class TRESULT>
void SeriesCollectTask<TRESULT>::attemptOperation(std::function<void(void)> func)
{
    try
    {
        func();
    }
    catch(...)
    {
        mPromise.set_exception(std::current_exception());
    }
}

}
}
}