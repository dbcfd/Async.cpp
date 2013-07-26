#pragma once
#include "async_cpp/async/detail/ISeriesTask.h"
#include "async_cpp/async/OpResult.h"

namespace async_cpp {
namespace async {
namespace detail {

//------------------------------------------------------------------------------
template<class TDATA>
class SeriesCollectTask : public ISeriesTask<TDATA> {
public:
    typedef typename std::function<void(AsyncResult&&)> callback_t;
    typedef typename std::function<void(OpResult<TDATA>&&, typename callback_t)> then_t;
    SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr, typename then_t thenFunc);
    virtual ~SeriesCollectTask();

    std::future<AsyncResult> future();
protected:
    virtual void performSpecific() final;
    virtual void notifyCancel() final;
    virtual void notifyException(const std::exception& ex) final;

private:
    typename then_t mThenFunc;
    std::promise<AsyncResult> mPromise;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
SeriesCollectTask<TDATA>::SeriesCollectTask(std::weak_ptr<tasks::IManager> mgr,
                                     typename then_t thenFunc)
                                     : ISeriesTask<TDATA>(mgr), 
                                     mThenFunc(thenFunc)
{

}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesCollectTask<TDATA>::~SeriesCollectTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesCollectTask<TDATA>::performSpecific()
{
    auto callback = [this](AsyncResult&& result)->void {
        mPromise.set_value(std::move(result));
    };

    mThenFunc(std::move(mPreviousResult), callback);
}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult> SeriesCollectTask<TDATA>::future()
{
    return mPromise.get_future();
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesCollectTask<TDATA>::notifyCancel()
{
    mPromise.set_value(AsyncResult(std::string("Cancelled")));
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesCollectTask<TDATA>::notifyException(const std::exception& ex)
{
    mPromise.set_value(AsyncResult(ex.what()));
}

}
}
}