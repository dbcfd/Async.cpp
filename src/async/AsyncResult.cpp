#include "async/AsyncResult.h"

namespace quicktcp {
namespace async {

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(const std::string& error) : mError(error)
{

}
    
//------------------------------------------------------------------------------
AsyncResult::AsyncResult(std::shared_ptr<void> data) : mResult(data)
{

}

//------------------------------------------------------------------------------
AsyncResult::AsyncResult()
{

}

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(AsyncResult&& other) : mError(std::move(other.mError)), mResult(std::move(other.mResult))
{

}

//------------------------------------------------------------------------------
AsyncResult::~AsyncResult()
{

}

//------------------------------------------------------------------------------
const bool AsyncResult::forwardError(std::shared_ptr<AsyncResult> other)
{
    other->mError = mError;
    return wasError();
}

//------------------------------------------------------------------------------
std::shared_ptr<void> AsyncResult::getOrThrowIfError() const
{
    throwIfError();
    return mResult;
}

//------------------------------------------------------------------------------
void AsyncResult::throwIfError() const
{
    if(wasError()) throw(std::runtime_error(mError));
}

//------------------------------------------------------------------------------
const bool AsyncResult::wasError() const
{
    return (mError.size() > 0);
}

}
}
