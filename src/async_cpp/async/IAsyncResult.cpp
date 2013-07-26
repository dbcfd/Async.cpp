#include "async_cpp/async/IAsyncResult.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
IAsyncResult::IAsyncResult(std::string&& error) : mError(new std::string(std::move(error)))
{
    if(mError->empty()) { throw(std::runtime_error("AsyncResult: Attempting to create error, with no message")); }
}

//------------------------------------------------------------------------------
IAsyncResult::IAsyncResult()
{

}

//------------------------------------------------------------------------------
IAsyncResult::IAsyncResult(IAsyncResult&& other) : mError(std::move(other.mError))
{

}

//------------------------------------------------------------------------------
IAsyncResult& IAsyncResult::operator=(IAsyncResult&& other)
{
     mError = std::move(other.mError);
     return *this;
}


//------------------------------------------------------------------------------
IAsyncResult::~IAsyncResult()
{

}

//------------------------------------------------------------------------------
void IAsyncResult::throwIfError() const
{
    if(wasError())
    {
        throw(std::runtime_error(*mError));
    }
}

//------------------------------------------------------------------------------
const bool IAsyncResult::wasError() const
{
    return (nullptr != mError);
}

//------------------------------------------------------------------------------
std::string IAsyncResult::error() const
{
    if(mError)
    {
        return *mError;
    }
    else
    {
        throw(std::runtime_error("IAsyncResult: Attempting to retrieve error message for result that is not an error"));
    }
}

}
}
