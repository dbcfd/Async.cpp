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
    virtual void notifyException(std::exception_ptr ex) final;
protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;

private:
    typename then_t mThenFunc;
    std::packaged_task<bool(std::exception_ptr, typename VariantType&&)> mTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TRESULT>
SeriesCollectTask<TRESULT>::SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr,
                                     typename then_t thenFunc)
                                     : ISeriesTask<TRESULT>(mgr), 
                                     mThenFunc(thenFunc)
{
    mTask = std::packaged_task<bool(std::exception_ptr, typename VariantType&&)>([thenFunc](std::exception_ptr ex, typename VariantType&& previous)->bool 
    {
        try
        {
            thenFunc(ex, boost::apply_visitor(ValueVisitor<TRESULT>(), previous));
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
SeriesCollectTask<TRESULT>::~SeriesCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TRESULT>
void SeriesCollectTask<TRESULT>::performSpecific()
{
    mTask(nullptr, std::move(mPreviousResult));
}

//------------------------------------------------------------------------------
template<class TRESULT>
AsyncResult SeriesCollectTask<TRESULT>::result()
{
    return AsyncResult(mTask.get_future());
}

//------------------------------------------------------------------------------
template<class T>
void SeriesCollectTask<T>::notifyCancel()
{
    mTask(std::make_exception_ptr(std::runtime_error("Cancelled")), std::move(mPreviousResult));
}

//------------------------------------------------------------------------------
template<class T>
void SeriesCollectTask<T>::notifyException(std::exception_ptr ex)
{
    mTask(ex, std::move(mPreviousResult));
}

}
}
}