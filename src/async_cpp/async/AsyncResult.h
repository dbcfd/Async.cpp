#pragma once
#include "async_cpp/async/Async.h"

#include <assert.h>
#include <future>
#include <memory>

namespace async_cpp {
namespace async {

/**
 * Store the result of an asynchronous operation, either as an error, or data.
 */
template<class TDATA>
class AsyncResult {
public:
    /**
     * Create a result that was an error
     * @param error String representing error
     */
    AsyncResult(const std::string& error);
    /**
     * Create a result that was valid, with data that will be retrieved later
     * @param data Shared point to data
     */
    AsyncResult(std::shared_ptr<TDATA> data);
    /**
     * Create a result that was valid, but you don't wish to return a result
     */
    AsyncResult();
    
    virtual ~AsyncResult();

    /**
     * Check if this result is an error, throwing a std::runtime_error if it is. If not an error,
     * return the result.
     */
    std::shared_ptr<TDATA> throwOrGet() const;
    /**
     * Throw a std::runtime_error if this result is an error
     */
    void throwIfError() const;
    /**
     * Indicate whether this result is an error.
     * @return True if this result is an error
     */
    const bool wasError() const;

    /**
     * Return the result held by this object
     * @return Result held
     */
    inline std::shared_ptr<TDATA> result() const;
    /**
     * Return the string that indicates errors
     * @return String holding error
     */
    inline const std::string& error() const;

    inline std::future<AsyncResult<TDATA>> asFulfilledFuture() const;

private:
    std::string mError;
    std::shared_ptr<TDATA> mResult;

};

//inline implementations
//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult<TDATA>::AsyncResult(const std::string& error) : mError(error)
{
    assert(error.size() > 0);
}
    
//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult<TDATA>::AsyncResult(std::shared_ptr<TDATA> data) : mResult(data)
{

}

//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult<TDATA>::AsyncResult()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
AsyncResult<TDATA>::~AsyncResult()
{

}

//------------------------------------------------------------------------------
template<class TDATA>
std::shared_ptr<TDATA> AsyncResult<TDATA>::throwOrGet() const
{
    throwIfError();
    return mResult;
}

//------------------------------------------------------------------------------
template<class TDATA>
void AsyncResult<TDATA>::throwIfError() const
{
    if(wasError()) throw(std::runtime_error(mError));
}

//------------------------------------------------------------------------------
template<class TDATA>
const bool AsyncResult<TDATA>::wasError() const
{
    return (mError.size() > 0);
}

//------------------------------------------------------------------------------
template<class TDATA>
std::shared_ptr<TDATA> AsyncResult<TDATA>::result() const
{
    return mResult;
}

//------------------------------------------------------------------------------
template<class TDATA>
const std::string& AsyncResult<TDATA>::error() const
{
    return mError;
}

//------------------------------------------------------------------------------
template<class TDATA>
std::future<AsyncResult<TDATA>> AsyncResult<TDATA>::asFulfilledFuture() const
{
    std::promise<AsyncResult> result;
    result.set_value(*this);
    return result.get_future();
}

}
}