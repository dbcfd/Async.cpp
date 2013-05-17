#pragma once
#include "async_cpp/async/Async.h"
#include "async_cpp/async/ISeriesTask.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
template<class TDATA>
class SeriesTerminalTask : public ISeriesTask<TDATA> {
public:
    SeriesTerminalTask(std::weak_ptr<tasks::IManager> mgr);
    SeriesTerminalTask(SeriesTerminalTask&& other);
    virtual ~SeriesTerminalTask();

    void cancel();
    std::future<AsyncResult<TDATA>> getFuture();

protected:
    virtual void performSpecific();
    virtual void notifyFailureToPerform();

private:    
    std::packaged_task<AsyncResult<TDATA>(AsyncResult<TDATA>&&)> mTask;
};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
SeriesTerminalTask<TDATA>::SeriesTerminalTask(std::weak_ptr<tasks::IManager> mgr) : ISeriesTask<TDATA>(mgr)
{
    mTask = std::packaged_task<AsyncResult<TDATA>(AsyncResult<TDATA>&&)>(
        [](AsyncResult<TDATA>&& forwardedResult) -> AsyncResult<TDATA> {
            return std::move(forwardedResult);
        }
    );
}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesTerminalTask<TDATA>::SeriesTerminalTask(SeriesTerminalTask&& other) 
    : ISeriesTask<TDATA>(std::move(other)),
    mTask(std::move(other.mTask))
{

}

//------------------------------------------------------------------------------
template<class TDATA>
SeriesTerminalTask<TDATA>::~SeriesTerminalTask()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTerminalTask<TDATA>::cancel()
{
    try {
        mTask(std::move(AsyncResult<TDATA>("Cancelled Task")));
        mIsCancelled = true;
    }
    catch(std::future_error&)
    {
        //task has complete successfully
    }
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTerminalTask<TDATA>::performSpecific()
{
    if(!mIsCancelled)
    {
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
        if(mForwardedFuture._Is_ready())
#else
        if(std::future_status::ready == mForwardedFuture.wait_for(std::chrono::milliseconds(0)))
#endif
        {
            try
            {
                mTask(std::move(mForwardedFuture.get()));
            }
            catch(std::future_error&)
            {
                //task was cancelled while we were checking future
            }
        }
        else
        {
            if(auto mgr = mManager.lock())
            {
                mgr->run(std::make_shared<SeriesTerminalTask<TDATA>>(std::move(*this)));
            }
            else
            {
                notifyFailureToPerform();
            }
        }
    }
}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult<TDATA>> SeriesTerminalTask<TDATA>::getFuture()
{
    return mTask.get_future();
}

//------------------------------------------------------------------------------
template<class TDATA>
void SeriesTerminalTask<TDATA>::notifyFailureToPerform()
{
    mTask(std::move(AsyncResult<TDATA>("SeriesTerminalTask: Failed to perform")));
}

}
}