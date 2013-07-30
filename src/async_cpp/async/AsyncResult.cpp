#include "async_cpp/async/AsyncResult.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(std::future<bool>&& future) 
    : mFuture(std::make_shared<std::future<bool>>(std::move(future)))
{

}

//------------------------------------------------------------------------------
AsyncResult::AsyncResult()
{
    std::promise<bool> promise;
    mFuture = std::make_shared<std::future<bool>>(promise.get_future());
    promise.set_value(true);
}


//------------------------------------------------------------------------------
AsyncResult::~AsyncResult()
{

}

//------------------------------------------------------------------------------
void AsyncResult::check()
{
    mFuture->get();
}

//------------------------------------------------------------------------------
bool AsyncResult::isReady() const
{
#ifdef _MSC_VER //wait_for is broken in VC11 have to use MS specific _Is_ready
    return mFuture->_Is_ready();
#else
    return (std::future_status::ready == mFuture->wait_for(std::chrono::milliseconds(0)));
#endif
}

}
}
