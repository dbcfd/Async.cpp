#include "async_cpp/async/AsyncResult.h"

namespace async_cpp {
namespace async {

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(std::string&& error) : IAsyncResult(std::move(error))
{

}

//------------------------------------------------------------------------------
AsyncResult::AsyncResult() : IAsyncResult()
{

}

//------------------------------------------------------------------------------
AsyncResult::AsyncResult(AsyncResult&& other) : IAsyncResult(std::move(other))
{

}

//------------------------------------------------------------------------------
AsyncResult& AsyncResult::operator=(AsyncResult&& other)
{
    IAsyncResult::operator=(std::move(other));
    return *this;
}

//------------------------------------------------------------------------------
AsyncResult::~AsyncResult()
{

}

//------------------------------------------------------------------------------
void AsyncResult::check() const
{
    throwIfError();
}

//------------------------------------------------------------------------------
bool AsyncResult::wasSuccessful() const
{
    return !wasError();
}

}
}
