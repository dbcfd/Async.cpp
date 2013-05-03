#include "async_cpp/async/AsyncResult.h"

#include <assert.h>

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(const std::string& error) : mError(error)
{
    assert(error.size() > 0);
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
AsyncResult::~AsyncResult()
{

}

//------------------------------------------------------------------------------
std::shared_ptr<void> AsyncResult::throwOrGet() const
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
